/********************************************************************************/
/*	har_debug.c								*/
/*	ハリアデバッグ関数群							*/
/*	2001/04/19  H.Satoyoshi							*/
/*	$Id: har_debug.c,v 1.1.1.3 2002/11/19 11:48:21 Yoshizawa1 Exp $		*/
/********************************************************************************/

#ifdef SATO_DEBUG
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void copypad							*/
/*	引数:	PAD_DATA *pad_d		コピー先				*/
/*		PAD_DATA *act_d		コピー元				*/
/*	説明:	パッドデータのコピー				    		*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void copypad(PAD_DATA *pad_d, PAD_DATA *act_d){
    pad_d->right_x = act_d->right_x;
    pad_d->right_y = act_d->right_y;
    pad_d->left_x = act_d->left_x;
    pad_d->left_y = act_d->left_y;
    pad_d->buton = act_d->buton;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void clearpaddata						*/
/*	引数:	PAD_DATA *pad_d						       	*/
/*	説明:	パッドワークをクリアする			 		*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void harclearpaddata(PAD_DATA *pad_d){
    pad_d->right_x = 128;
    pad_d->right_y = 128;
    pad_d->left_x = 128;
    pad_d->left_y = 128;
    pad_d->buton = 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CopyPadData						*/
/*	引数:	PAD_DATA *pad_d						       	*/
/*	説明:	読みとりパッドワークにコントローラのデータをコピーする 		*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void copypaddata(PAD_DATA *pad_d){
    pad_d->right_x = GV_PadData[0].right_dx;
    pad_d->right_y = GV_PadData[0].right_dy;
    pad_d->left_x = GV_PadData[0].left_dx;
    pad_d->left_y = GV_PadData[0].left_dy;
    pad_d->buton = GV_PadDataDirect[0].press;
}


/*******************************<Global function>********************************/
/*	名前:	void *Har_StageChange						*/
/*	引数:	Work* work							*/
/*	説明:	ステージ変更	      						*/
/********************************************************************************/
void Har_StageChange( Work* work )
{

    printf ("VCAll\n");
    if ( work->brake_proc_id != 0 ){
	GCL_ARGS	args;
	FVECTOR		point={-1250.0f, -1000.0f, -154000.0f, 1.0f};
	FVECTOR		normal={0.0f, 1.0f, 0.0f, 0.0f};
	FVECTOR		direction={0.0f, 0.0f, -1.0f, 0.0f};

	args.argc = 1;
	args.argv = &work->name;
	GCL_ExecProc(work->brake_proc_id, &args);

	GM_SeSetMode (SD_W_EXPLOS02, &point, GM_SEMODE_BOMB);
	NewHarrierMisileExplosionE3(
	    &point,
	    &normal,
	    &direction );
	
	printf ("Stage Brake proc Call. ID = %d\n", work->name);
    }
}

/*******************************<Global function>********************************/
/*	名前:	void Har_DBControlVTOL						*/
/*	引数:	Work *work   						       	*/
/*	説明:	ハリアのVTOL時の操作			       			*/
/********************************************************************************/
void Har_DBControlVTOL(Work *work)
{
    
    // *****ピッチ
    work->control.turn.vx = work->control.turn.vx * 0.8;
    work->control.turn.vx += work->zenkei_rot;
    
    // *****上昇下降
    if (LEFT_ANALOGY > 30){
	if (work->control.step.vy > -LEFT_ANALOGY *25/10){	//限界角度
	    work->control.step.vy = work->control.step.vy - 5;
	}
	else {
	    work->control.step.vy = -LEFT_ANALOGY *25/10;
	}
    }
    else if (LEFT_ANALOGY < -30){
	if (work->control.step.vy < -LEFT_ANALOGY *25/10){	//限界角度
	    work->control.step.vy = work->control.step.vy + 5;
	}
	else {
	    work->control.step.vy = -LEFT_ANALOGY *25/10;
	}
    }
    else {
	work->control.step.vy = work->control.step.vy * 0.95f;
    }
    
    
    // *****傾き方向
    if (LEFT_ANALOGX > 20){	//右向く
	work->control.turn.vy = work->control.turn.vy - LEFT_ANALOGX/18;
	if (work->lader_rot < 0.2f){
	    work->lader_rot = work->lader_rot + 0.06f; //ラダーを動かす
	}
    }
    else if (LEFT_ANALOGX < -20){
	work->control.turn.vy = work->control.turn.vy - LEFT_ANALOGX/18;
	if (work->lader_rot > -0.2f){
	    work->lader_rot = work->lader_rot - 0.06f; //ラダー動き
	}
    }
    else {
	work->lader_rot = work->lader_rot * 0.8f; 
    }
    
    
    // *****傾き  ロール
    if (RIGHT_ANALOGX > 30){
	if (work->control.turn.vz < RIGHT_ANALOGX *25/10){			//限界角度
	    work->control.turn.vz = work->control.turn.vz + 5;
	}
	else {
	    work->control.turn.vz = RIGHT_ANALOGX *25/10;
	}
	if (work->l_elr_rot > -0.4f){
	    work->l_elr_rot = work->l_elr_rot - 0.06f;
	    work->r_elr_rot = work->r_elr_rot - 0.06f;
	}
    }
    else if (RIGHT_ANALOGX < -30){
	if (work->control.turn.vz > RIGHT_ANALOGX *23/10){			//限界角度
	    work->control.turn.vz = work->control.turn.vz - 5;
	}
	else {
	    work->control.turn.vz = RIGHT_ANALOGX *23/10;
	}
	if (work->l_elr_rot < 0.4f){
	    work->l_elr_rot = work->l_elr_rot + 0.06f;
	    work->r_elr_rot = work->r_elr_rot + 0.06f;
	}
    }
    else {
	work->control.turn.vz = work->control.turn.vz * 0.95f;
	work->l_elr_rot = work->l_elr_rot * 0.93f;
	work->r_elr_rot = work->r_elr_rot * 0.93f;
    }
    
    
    // *****推力
    if (RIGHT_ANALOGY < -30){
	if (work->front_power < -RIGHT_ANALOGY *35.0f/10.0f){	//限界角度
	    work->front_power = work->front_power + 5.0f;
	}
	else {
	    work->front_power = -RIGHT_ANALOGY *35.0f/10.0f;
	}
	work->r_flap_rot = work->r_flap_rot * 0.96;
	work->noz_fr_rot = work->noz_fr_rot * 0.96;
    }
    else {
	work->front_power = work->front_power * 0.96f;
	if (RIGHT_ANALOGY > 30){
	    if (work->front_power > -RIGHT_ANALOGY *35.0f/10.0f){	//限界角度
		work->front_power = work->front_power - 5.0f;
	    }
	    else {
		work->front_power = -RIGHT_ANALOGY *35.0f/10.0f;
	    }
	}
	else {
	    if (work->r_flap_rot < 0.7){
		work->r_flap_rot = work->r_flap_rot + 0.017f;
	    }
	    // *****ノズル角度
	    if (work->noz_fr_rot > -(work->control.turn.vx+1024)*M_PI/4096 ){
		work->noz_fr_rot = work->noz_fr_rot - 0.01f;
	    }
	    else if (work->noz_fr_rot < -(work->control.turn.vx+1024)*M_PI/4096 - 0.011){
		work->noz_fr_rot = work->noz_fr_rot + 0.01f;
	    }

	}
    }    
    
    //前進のスピード
    work->control.step.vz = work->front_power * cosf(M_PI*work->control.rot.vy/2048.0f);
    work->control.step.vx = work->front_power * sinf(M_PI*work->control.rot.vy/2048.0f);
    
    //横滑べりのスピード
    work->control.step.vx -= 400.0f *sinf(M_PI*work->control.rot.vz/2048.0f)
	* cosf(M_PI*work->control.rot.vy/2048.0f);
    work->control.step.vz += 400.0f *sinf(M_PI*work->control.rot.vz/2048.0f)
	* sinf(M_PI*work->control.rot.vy/2048.0f);
}

/*******************************<Global function>********************************/
/*	名前:	void Har_DBMoveCamera						*/
/*	引数:	Work *work   						       	*/
/*	説明:	カメラ移動				       			*/
/********************************************************************************/
void Har_DBMoveCamera(Work *work)
{
    
    FVECTOR	from, to;
    FMATRIX	tmpmat;
    FVECTOR	from_shift = { 0.0f, 0.0f, 0.0f, 0.0f};
    FVECTOR	to_shift = { 0.0f, 1500.0f, 50000.0f, 0.0f};
    
    if (work->camera_flag == 1){
	from_shift.vy = 2800.0f;
	from_shift.vz = -9000.0f;
    }
    else if (work->camera_flag == 2){
	from_shift.vy = 2000.0f;
	from_shift.vz = 2500.0f;
    }
    
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &from_shift );
    DG_GetPos( &tmpmat );
    Fvec_from_Matrix( &from, &tmpmat );
    
    DG_SetPos( &work->body.objs->objs[0].world);
    DG_MovePos( &to_shift );
    DG_GetPos( &tmpmat );
    Fvec_from_Matrix( &to, &tmpmat );
    
    
    // カメラを一切補間しないように値を更新する */
    GM_SetCameraQuick( 0 );
    
    work->camera->position = from ;
    work->camera->target = to ;
    work->camera->angle = 1.70f ;
    work->camera->rotate.vz = -work->control.rot.vz;
    
}


/*******************************<Global function>********************************/
/*	名前:	void Har_DBPCPadMove						*/
/*	引数:	Work *work   						       	*/
/*	説明:	パッドによる操作			       			*/
/********************************************************************************/
void Har_DBPCPadMove(Work *work)
{
    GM_ResetPlayerStatus( PLAYER_PAD_OFF );
    
    //    GV_PadData[ 0 ].flag &= ~GV_PAD_RELEASE_SCN;//操作受け付け
    
    if (work->menu_flag&HAR_DEB_PLAY_FLAG){
	
	//	**************再生関連****************************************

#ifdef SAT
	if (GV_PadDataDirect[1].press & PAD_B){
	    if (work->play_flag){	
		work->play_flag = OFF;
		harclearpaddata(&work->act_work);
	    }
	    else {
		work->play_flag = ON;
		work->hokan_timer = 128;
	    }
	    MACRO_HARRIER_STOP;
	    work->play_timer = 0;
	    harclearpaddata(&work->act_work);
	}
#endif
	
	if (work->play_flag == 1){
	    if (work->har_flag & HAR_DOUSA_FLAG_DEBUG){
		if (work->play_timer%80 < 50){
		    MENU_Locate( 250, 425, 0 );
		    MENU_SetColor( 200, 0, 0 );
		    //		    MENU_Printf( "Now Playing !"); 
		}
		MENU_Locate( 430, 425, 0 );
		MENU_SetColor( 0, 0, 200 );
		//		MENU_Printf( "[%d]",work->play_timer ); 
	    }
	}
	else {
	    if (work->har_flag & HAR_DOUSA_FLAG_DEBUG){
		MENU_Locate( 225, 425, 0 );
		MENU_SetColor( 0, 0, 200 );
		//		MENU_Printf( "Press X to start play"); 
	    }
	}
    }
    
    if (work->play_flag){	//再生
	if (work->hokan_timer == 0){
	    Har_PlayAction(work, 0);
	}
    }
    
}


/*******************************<Global function>********************************/
/*	名前:	void Har_DBHarPadMove						*/
/*	引数:	Work *work   						       	*/
/*	説明:	パッドによる操作			       			*/
/********************************************************************************/
void Har_DBHarPadMove(Work *work)
{
    GM_SetPlayerStatus( PLAYER_PAD_OFF );    
    //    GV_PadData[ 0 ].flag |= GV_PAD_RELEASE_SCN;	//プレイヤ操作受け付けせず
    
    if (work->menu_flag&HAR_DEB_RECORD_FLAG){
	//	*************録画*************************************************
	if (GV_PadDataDirect[1].press & PAD_B){
	    if (work->record_flag){
		work->record_flag = 0;
		work->har_move.p_data[work->play_timer].buton = HAR_ACTION_END;
	    }
	    else {
		work->record_flag = 1;
	    }
	    MACRO_HARRIER_STOP;
	    harclearpaddata(&work->act_work);
	    work->play_timer = 0;
	}
	
	if (work->record_flag == 1){
	    if (work->play_timer%80 < 50){
		MENU_Locate( 250, 425, 0 );
		MENU_SetColor( 200, 0, 0 );
		MENU_Printf( "Now Recording !"); 
	    }
	    MENU_Locate( 430, 425, 0 );
	    MENU_SetColor( 200, 0, 0 );
	    MENU_Printf( "[%d]",work->play_timer ); 
	    
	    
	    //パッドデータの記録
	    copypad(&work->har_move.p_data[work->play_timer], &work->act_work);
	    
	    if (work->har_flag & HAR_DOUSA_FLAG_DEBUG){
		MENU_Locate( 400, 50, 0 ) ;
		MENU_SetColor( 200, 0, 0 ) ;
		MENU_Printf( "PLAY"); 
	    }
	    
	    
	    //		開始位置の保存
	    if (work->play_timer == 0){
		_sceVu0CopyVector(&work->har_move.begin_pos, &work->control.mov);
		work->har_move.begin_rot.vx = work->control.rot.vx;
		work->har_move.begin_rot.vy = work->control.rot.vy;
		work->har_move.begin_rot.vz = work->control.rot.vz;
		
	    }
	    
	    if (work->play_timer<HAR_MOVE_FRAMES){
		work->play_timer++;
	    }
	    else {
		MACRO_HARRIER_STOP;
		MENU_Locate( 400, 50, 0 ) ;
		MENU_SetColor( 200, 0, 0 ) ;
		MENU_Printf( "          - END"); 
	    }
	    
	    
	}
	else {
	    if (work->har_flag & HAR_DOUSA_FLAG_DEBUG){
		MENU_Locate( 225, 425, 0 );
		MENU_SetColor( 180, 0, 0 );
		MENU_Printf( "Press X to start recording"); 
	    }
	    work->record_flag = 0;
	}
    }

#if 0    
    if (GV_PadDataDirect[0].press & PAD_STA){	//ポーズをかけるぞ
	if (GV_PauseLevel & GV_PAUSE_PAUSE){
	    GV_PauseOffActorSystem( GV_PAUSE_PAUSE );
	}
	else {
	    GV_PauseOnActorSystem( GV_PAUSE_PAUSE );
	}
    }
#endif

    //パッドデータのコピー
    copypaddata(&work->act_work);
}




#define	HAR_MENU_NUM	8

#define	SELECT_GREEN	100
#define	SELECT_BLUE	150

#define PAD_1_2 (GV_PadData[0].press | GV_PadData[1].press) 

/*******************************<Global function>********************************/
/*	名前:	void Har_DBPrintMenu						*/
/*	引数:	Work	*work							*/
/*	説明:	デバッグ用メニュー表示						*/
/********************************************************************************/
void Har_DBPrintMenu(Work *work)
{
    short		loop;
    static short	select;
    char	fname[30][63] = {
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp00.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp01.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp02.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp03.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp04.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp05.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp06.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp07.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp08.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp09.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp10.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp11.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp12.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp13.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp14.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp15.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp16.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp17.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp18.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp19.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp20.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp21.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp22.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp23.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp24.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp25.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp26.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp27.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp28.dat",
	"host0:/usr/local/develop/mgs2/source/user/satoyoshi/tmp29.dat",
    };
    
    

#ifdef SAT    
    //セレクトボタンは メニューウィンドウ呼びだし
    if (PAD_1_2 & PAD_AL){
	if (work->menu_flag & HAR_DEB_MENU_OPEN){	//メニューを閉じる
	    work->menu_flag &= ~HAR_DEB_MENU_OPEN;	
	    GM_ResetPlayerStatus( PLAYER_PAD_OFF );//プレイヤ操作不能	
	}
	else {						//メニューを開く
	    work->menu_flag |= HAR_DEB_MENU_OPEN;	
	    GM_SetPlayerStatus( PLAYER_PAD_OFF );//プレイヤ操作可能	
	    work->record_flag = 0;
	    work->play_flag = 0;
	    MACRO_HARRIER_STOP;

	    work->har_mode = HAR_MODE_NORMAL;
	}
    }
#endif
    
    
    if (work->menu_flag & HAR_DEB_MENU_OPEN){	//メニュー表示
	short	loop2;
	char	menu_name[HAR_MENU_NUM][16] = {"Move Harrier",
					       "Camera Set",
					       "Record Action",
					       "Play Action",
					       "Stage Brake",
					       "Debug Mode",
					       "HDD Save Data ",
					       "Read Data"};
	
	//２コンパッドで メニュー操作
	if (PAD_1_2 & PAD_D){
	    select ++;
	    if (select >= HAR_MENU_NUM ){
		select = 0;
	    }
	}
	if (PAD_1_2 & PAD_U){
	    select --;
	    if (select < 0 ){
		select = HAR_MENU_NUM-1;
	    }
	}
	
	MENU_Locate( 160, 100, 0 ) ;
	MENU_SetColor( 0, 0, 200 ) ;
	MENU_Printf( "* Harrier Debug Menu *"); 
	
	for (loop=0; loop<HAR_MENU_NUM; loop++){
	    
	    if (select == loop){	//選択されているメニュー色変える
		MENU_SetColor( 0, SELECT_GREEN, 0 );
	    }
	    else {
		MENU_SetColor( 0, 0, SELECT_BLUE );
	    }
	    
	    MENU_Locate( 90, 150+20*loop, 0 ) ;
	    MENU_Printf( menu_name[loop] ); 
	    
	    
	    switch (loop){
		
	    case 0:	//ハリアを操作しますか？
	    {
		char	select_name[3][4] = {"ON",
					     "OFF"};
		if (select == loop){
		    if (PAD_1_2 & PAD_R){	//操作オフ
			work->menu_flag &= ~HAR_DEB_CONTROL_FLAG;
			work->menu_flag &= ~HAR_DEB_RECORD_FLAG;
		    }
		    if (PAD_1_2 & PAD_L){	//操作オン
			work->menu_flag |= HAR_DEB_CONTROL_FLAG;
			work->menu_flag &= ~HAR_DEB_PLAY_FLAG;	
		    }
		}
		for (loop2=0; loop2<2; loop2++){
		    //選択されているメニューは色変える
		    if (((work->menu_flag&HAR_DEB_CONTROL_FLAG)==HAR_DEB_CONTROL_FLAG) == loop2){
			MENU_SetColor( 0, 0, SELECT_BLUE );
		    }
		    else {
			MENU_SetColor( 0, SELECT_GREEN, 0 );
		    }
		    MENU_Locate( 250+loop2*30, 150+20*loop, 0 );
		    MENU_Printf(select_name[loop2]); 
		}
	    }
	    break;
	    
	    case 1:	//カメラ位置の選択
	    {
		char	select_name[3][8] = {"Normal",
					     "Behind",
					     "Inside"};
		if (select == loop){	//オプションの選択
		    if (PAD_1_2 & PAD_R){
			work->camera_flag ++;
			if (work->camera_flag >= 3 ){
			    work->camera_flag = 0;
			}
			//カメラの設定
			if (work->camera_flag){
			    work->camera->on = 1 ;
			}
			else {
			    work->camera->on = 0 ;
			}
			GM_ChangeCamera( 0 );
		    }
		    if (PAD_1_2 & PAD_L){
			work->camera_flag --;
			if (work->camera_flag < 0 ){
			    work->camera_flag = 2;
			}
			//カメラの設定
			if (work->camera_flag){
			    work->camera->on = 1 ;
			}
			else {
			    work->camera->on = 0 ;
			}
			GM_ChangeCamera( 0 );
		    }
		}
		for (loop2=0; loop2<3; loop2++){
		    if (work->camera_flag == loop2){	//選択されているメニューは色変える
			MENU_SetColor( 0, SELECT_GREEN, 0 );
		    }
		    else {
			MENU_SetColor( 0, 0, SELECT_BLUE );
		    }
		    MENU_Locate( 250+loop2*70, 150+20*loop, 0 );
		    MENU_Printf(select_name[loop2]); 
		}
	    }
	    break;
	    
	    case 2:	//アクションの記録
	    {
		char	select_name[2][4] = {"ON",
					     "OFF"};
		if (select == loop){	//オプションの選択
		    if (PAD_1_2 & PAD_R){	//記録OFF
			work->menu_flag &= ~HAR_DEB_RECORD_FLAG;	
		    }
		    if (PAD_1_2 & PAD_L){	//記録オン
			work->menu_flag |= HAR_DEB_RECORD_FLAG;	
			work->menu_flag &= ~HAR_DEB_PLAY_FLAG;	//再生オフ
			work->menu_flag |= HAR_DEB_CONTROL_FLAG;	//操作オン
		    }
		}
		for (loop2=0; loop2<2; loop2++){
		    //選択されているメニューは色変える
		    if (((work->menu_flag&HAR_DEB_RECORD_FLAG)==HAR_DEB_RECORD_FLAG) == loop2){
			MENU_SetColor( 0, 0, SELECT_BLUE );
		    }
		    else {
			MENU_SetColor( 0, SELECT_GREEN, 0 );
		    }
		    MENU_Locate( 250+loop2*30, 150+20*loop, 0 );
		    MENU_Printf(select_name[loop2]); 
		}
	    }
	    break;
	    
	    case 3:	//アクションの再生
	    {
		char	select_name[2][4] = {"ON",
					     "OFF"};
		if (select == loop){	//再生オフ
		    if (PAD_1_2 & PAD_R){
			work->menu_flag &= ~HAR_DEB_PLAY_FLAG;	
		    }
		    if (PAD_1_2 & PAD_L){	//再生オン
			work->menu_flag |= HAR_DEB_PLAY_FLAG;	
			work->menu_flag &= ~HAR_DEB_CONTROL_FLAG;//コントロールOFF
			work->menu_flag &= ~HAR_DEB_RECORD_FLAG;//記録OFF	
		    }
		}
		for (loop2=0; loop2<2; loop2++){
		    //選択されているメニューは色変える
		    if (((work->menu_flag&HAR_DEB_PLAY_FLAG)==HAR_DEB_PLAY_FLAG) == loop2){
			MENU_SetColor( 0, 0, SELECT_BLUE );
		    }
		    else {
			MENU_SetColor( 0, SELECT_GREEN, 0 );
		    }
		    MENU_Locate( 250+loop2*30, 150+20*loop, 0 );
		    MENU_Printf(select_name[loop2]); 
		}
	    }
	    break;
	    
	    
	    case 4:	//ステージ破壊
	    {
		if ((PAD_1_2 & PAD_A) && (select == loop)){
		    Har_StageChange( work );
		}
	    }
	    break;
	    
	    case 5:	//デバッグ表示
	    {
		char	select_name[2][4] = {"ON",
					     "OFF"};
		if (select == loop){	//オプションの選択
		    if (PAD_1_2 & PAD_R){	//記録OFF
			work->har_flag &= ~HAR_DOUSA_FLAG_DEBUG;
		    }
		    if (PAD_1_2 & PAD_L){	//記録オン
			work->har_flag |= HAR_DOUSA_FLAG_DEBUG;
		    }
		}
		for (loop2=0; loop2<2; loop2++){
		    //選択されているメニューは色変える
		    if (((work->har_flag & HAR_DOUSA_FLAG_DEBUG)==HAR_DOUSA_FLAG_DEBUG) == loop2){
			MENU_SetColor( 0, 0, SELECT_BLUE );
		    }
		    else {
			MENU_SetColor( 0, SELECT_GREEN, 0 );
		    }
		    MENU_Locate( 250+loop2*30, 150+20*loop, 0 );
		    MENU_Printf(select_name[loop2]); 
		}
	    }
	    break;
	    
	    
	    case 6:	//アクションデータHDDへの書き込み
	    {
		static short	select_data = 0;
		
		if (select == loop){
		    if (PAD_1_2 & PAD_R){	//記録OFF
			if (select_data < 29){
			    select_data++;
			}
		    }
		    if (PAD_1_2 & PAD_L){	//記録OFF
			if (select_data > 0){
			    select_data--;
			}
		    }
		}
		
		
		if ((PAD_1_2 & PAD_A) && (select == loop)){
		    int	fd;
		    int	status;
		    
		    fd = sceOpen(fname[select_data], SCE_WRONLY|SCE_CREAT|SCE_TRUNC);
		    if (fd >= 0){
			printf("Now Saving to HDD:%d   DATA[ %d ]\n", fd, select_data);
			
			status = sceWrite ( fd, &work->har_move, sizeof(HAR_MOVE) );
			
			printf("End!! : %d\n", status);
		    }
		    else {
			printf("Error Can't Save Action Data : %d\n", fd);
		    }
		    
		    sceClose(fd);
		}
		
		MENU_SetColor( 0, 0, SELECT_BLUE );
		MENU_Locate( 250, 150+20*loop, 0 );
		MENU_Printf( "Push O to write DATA [ %d ]",select_data); 
	    }
	    break;
	    
	    case 7:	//アクションデータHDDからの読み込み
	    {
		static short	select_data = 0;
		
		if (select == loop){
		    if (PAD_1_2 & PAD_R){	//記録OFF
			if (select_data < 29){
			    select_data++;
			}
		    }
		    if (PAD_1_2 & PAD_L){	//記録OFF
			if (select_data > 0){
			    select_data--;
			}
		    }
		}
		
		if ((PAD_1_2 & PAD_A) && (select == loop)){
		    int	fd;
		    int	status;
		    fd = sceOpen(fname[select_data], SCE_RDONLY);
		    if (fd >= 0){
			printf("Now Reading from HDD:%d   DATA[ %d ]\n", fd, select_data);
			status = sceRead ( fd, &work->har_move, sizeof(HAR_MOVE) );			
			printf("End!! : %d\n", status);
		    }
		    else {
			printf("Error Can't Read Action Data : %d\n", fd);
		    }
		    sceClose(fd);
		}
		
		MENU_SetColor( 0, 0, SELECT_BLUE );
		MENU_Locate( 250, 150+20*loop, 0 );
		MENU_Printf( "Push O to read DATA [ %d ]",select_data); 
	    }
	    break;
	    
	    }
	    
	    
	}
	
    }
    
}



/*******************************<Global function>********************************/
/*	名前:	void Har_DBPrintMenu						*/
/*	引数:	Work	*work							*/
/*	説明:	デバッグ用メニュー表示						*/
/********************************************************************************/
void RUN_ACTION (Work *work, char *action)
{
    work->act_move = (HAR_MOVE*)action;    

    work->menu_flag |= HAR_DEB_PLAY_FLAG;	
    work->menu_flag &= ~HAR_DEB_CONTROL_FLAG;//コントロールOFF
    work->menu_flag &= ~HAR_DEB_RECORD_FLAG;//記録OFF	
    
    work->play_flag = ON;


    {
	FMATRIX tmpmat;
	DG_SetPos2 (&work->act_move->begin_pos, &work->act_move->begin_rot);
	DG_GetPos (&tmpmat);
	work->hokan_timer = 400;
	Har_SetHokanMove(work, &tmpmat, 400);
	printf ("Run Action\n");

    }

    work->har_act_suki = 0;	//すきカウンタークリア

    MACRO_HARRIER_STOP;
    work->play_timer = 0;
    harclearpaddata(&work->act_work);
    har_call_event_proc(work, HAR_EVENT_RM_START );
}

#endif






