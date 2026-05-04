/********************************************************************************/
/*	cypher.c								*/
/*	サイファ	*NewCypher						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cyp_act.c,v 1.1.1.3 2002/11/19 11:48:14 Yoshizawa1 Exp $			*/
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

#ifdef KP_XBOX	// サイファ音は遮蔽計算なしにする
#define	GM_SeSetFromVolCurve( a, b, c ) GM_SeSetFromVolCurveAddr( a, b, c, GM_INVALID_ADDR )
#define GM_SeSetMode( a, b, c ) GM_SeSetModeAddr( a, b, c, GM_INVALID_ADDR )
#endif

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cyp_norm_act						*/
/*	引数:	Work *work   						       	*/
/*	説明:	実行関数				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_norm_act(Work *work){
    //    short	tmp_uniq_id;
    //    short	loop;
    switch(work->mode){
	
    case MOVE_MODE :	//移動モード
	MoveRoute(work);
	break;
	
    case ACTION_MODE :	//待機場所へ補正
	if(DirectTrace3D(work,1000.0F,NULL)<0){
	}
	ActionSeq(work);
	break;
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cyp_chaff							*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフのチェックとかその他		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_act_chaff(Work *work){
    if (GM_CheckGameStatus(STATE_CHAFF)){
	int temp;

	temp = RAND(101)-50;

	work->control.turn.vx += temp;
	if (abs(temp) > 48){
	    GM_SeSetFromVolCurve( work->chaf_se_num ,&work->control.mov, &cyp_curves);
	}

	temp = RAND(201)-100;

	work->control.turn.vy += temp;
	if (abs(temp) > 96){
	    GM_SeSetFromVolCurve( work->chaf_se_num ,&work->control.mov, &cyp_curves);
	}


	//	printf ("R: %d", work->control.turn.vx);
    }
    //    if (chaff_timer <= 0){
    else {
	SV_G1_STEP(work, CYP_NORMAL);
	work->ACTION_LAYER = EYE_LAYER_Normal;
    }
    DecayStep(&work->control,0.85f);
}	



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int SV_Chaff_Check		** EYE_LAYER_CHAFF **		*/
/*	引数:	Work	*work							*/
/*	説明:	チャフチェック							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int CYP_Chaff_Check(Work *work){
    
    if(GM_CheckGameStatus(STATE_CHAFF)){	// チャフチェック
	if (work->ACTION_LAYER > EYE_LAYER_CHAFF){
	    SV_G1_STEP(work, CYP_CHAFF);
	    work->ACTION_LAYER = EYE_LAYER_CHAFF;
	    return 1;
	}
    }
    return 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void act_stall							*/
/*	引数:	Work *work   						       	*/
/*	説明:	落下挙動				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_act_stall(Work *work)
{


    // サイファーダメージプラズマ
    if (RAND(18)== 0)
    {	
	FVECTOR color;
	
	color.vx = 32.0f;
	color.vy = 100.0f;
	color.vz = 190.0f;
	color.vw = 64.0f;
	
	GV_SetActorChild( work, NewCypherPlasma(&work->body.objs->objs[0].world, 30.0f, &color));
	GM_SeSetMode( SD_E_HIBANA02, &work->control.mov, GM_SEMODE_BOMB );
    }
    
    if (work->dmg_flag == 1){
	work->smokep = NewCypherRisingSmoke( (FVECTOR*)work->damage_pos.m[3], 200.0f );
	if (work->smokep != NULL){
	    GV_SetActorChild( work, work->smokep);
	    work->dmg_flag = 2;
	}
    }

#if 0
    if( GV_CheckMemory() == 0 ){
	printf( "03\n");
	HANGUP();
    }
#endif
    
    if (work->dmg_flag == 2){
	FMATRIX	tmpmat;
	FVECTOR	tmpvec = {0.0f, 0.0f, 20.0f, 1.0f};
	_sceVu0MulMatrix(&work->damage_pos, &work->body.objs->objs[0].world, &work->damage_shift);
	
	//  ****落下していく
	{
	    work->nigeru_dir.vy += 100-RAND(201);
	    
	    DG_SetPos2(&DG_ZeroVector, &work->nigeru_dir);
	    DG_MovePos(&tmpvec);
	    DG_GetPos(&tmpmat);
	    
	    _sceVu0CopyVector(&work->nigeru_vec, tmpmat.m[3]);

	    
	    work->control.step.vx = work->nigeru_vec.vx*(RAND(30)+10)/15.0f;
	    work->control.step.vz = work->nigeru_vec.vz*(RAND(30)+10)/15.0f;
	    work->control.step.vy = 20.0f+RAND(20)*1.0f;

	    work->tuiraku_timer += TIME_BASE;

	    //	    printf ("%d\n", work->tuiraku_timer);

	    work->rot_body.vy += work->tuiraku_timer*3+100;
	    work->rot_body.vy %= (4096*5);

	    work->control.step.vy -= work->tuiraku_timer*0.08f;

	    work->rot_body.vx = work->tuiraku_timer*8;
	    if (work->rot_body.vx > 900){
		work->rot_body.vx = 900;
	    }

	    if (work->tuiraku_timer > 700){
		CypherBrake( work, 0 );
		work->dmg_flag = 3;
		return;
	    }
	    
	    
	    if (work->control.mov.vy < -50000.0f){
		if (work->act_end_time == 0){
		    CypherBrake( work, 0 );
		    work->dmg_flag = 3;
		    return;
		}
	    }
	}

	if (work->dmg_flag != 3){
	    
	    // **高さチェックあり
	    if ( HZX_LevelHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
				       &work->control.mov, HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE,
				       HZX_SEG_ALL) ){
//	    printf("N *Check %x  %x\n",GM_CurrentStageMap ,GM_GetHzxGroupID( GM_CurrentStageMap ) );
	    //	    sat_viewp(&work->control.mov, 250, 0, 0);
		if ( (fabs(work->control.mov.vy-HZX_GetFloorLevel())<1000.0f)  ||
		     (HZX_NearHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
					   &work->control.mov, 1000.0f, HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE,
					   HZX_SEG_ALL, 1000.0f)) ){
		    CypherBrake( work, 0 );
		    work->dmg_flag = 3;
		}
	    }
	    else {
//	    printf("N +Check %x  %x\n",GM_CurrentStageMap ,GM_GetHzxGroupID( GM_CurrentStageMap ) );
		if ( (HZX_NearHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
					   &work->control.mov, 1000.0f, HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE,
					   HZX_SEG_ALL, 1000.0f)) ){
		    CypherBrake( work, 0 );
		    work->dmg_flag = 3;
		}
	    }
	}
    }
}
/**************************<-------local function------>*************************/
/*	名前:	void DEV_CameraSe						*/
/*	返値:	なし								*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	首振り効果音を鳴らす						*/
/********************************************************************************/
void CYP_CameraSe(Work *work){
    if (G1_STEP != CYP_CHAFF){
	/*首振り効果音*/
	if((DG_TickCount - work->se_tick)*TIME_BASE > CAM_SE_INT*20){	//音を鳴らした瞬間を保存
	    GM_SeSetFromVolCurve( work->turn_se_num ,&work->control.mov, &cyp_curves);
	    work->se_tick = DG_TickCount ;
	}
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cyp_sound_cnt						*/
/*	引数:	Work *work   						       	*/
/*	説明:	サイファーの音鳴らし			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_sound_cnt(Work *work)    // ****ＳＥを鳴らします
{
    //    int volume, pan;
    //    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &cyp_curves);



    if (G1_STEP==CYP_DAMAGE){
	if (work->se_timer >= 15*5){
	    work->se_timer -= 75;
	    GM_SeSetFromVolCurve( work->yota_se_num, &work->control.mov, &cyp_curves);
	}
    }
    else {
	if (work->se_timer >= 6*5){
	    work->se_timer -= 30;

	    //	    printf ("ST %f\n",work->control.step.vy);
	    
	    if (work->control.step.vy > 15.0f){
		if (work->fin_counter<3){
		    GM_SeSetFromVolCurve( work->tugi_se_num, &work->control.mov, &cyp_curves);
		    work->fin_counter ++;
		}
		else {
		    GM_SeSetFromVolCurve( work->move_se_num, &work->control.mov, &cyp_curves);
		}
	    }
	    else {
		if (work->fin_counter>0){
		    GM_SeSetFromVolCurve( work->tugi_se_num, &work->control.mov, &cyp_curves);
		    work->fin_counter --;
		}
		else {
		    GM_SeSetFromVolCurve( work->still_se_num, &work->control.mov, &cyp_curves);
		}
	    }
	}
    }
    work->se_timer += TIME_BASE;

    /*駆動音は同一マップのみ*/
    if( work->control.map == GM_PlayerMap ){
	if( (work->rot_vy_buf != work->control.rot.vy) ||
	    (work->rot_vx_buf != work->control.rot.vx) ){
	    CYP_CameraSe(work);
	}
    }
    work->rot_vx_buf = work->control.rot.vx;
    work->rot_vy_buf = work->control.rot.vy;

}








