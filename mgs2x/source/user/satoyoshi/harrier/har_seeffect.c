/********************************************************************************/
/*	har_seeffect.c								*/
/*	ハリアエフェクト							*/
/*	2001/04/19 H.Satoyoshi							*/
/*	$Id: har_seeffect.c,v 1.1.1.3 2002/11/19 11:48:24 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#include "BP_Misc.h"

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void engine_se							*/
/*	引数:	Work	*work							*/
/*	説明:	ハリアのエンジン音 ＳＥ						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void har_burning_se(Work *work){

    if (work->har_flag & HAR_DOUSA_FLAG_BOSSMD){
	if (work->se_time == 0){
	    GM_SeSetMode (SD_E_NEPPOST1, &work->control.mov, GM_SEMODE_BOMB); //SEハリア熱風開始
	}
	if (work->se_time > 125){
	    GM_SeSetMode (SD_E_NEPPOLP1, &work->control.mov, GM_SEMODE_BOMB); //SEハリア熱風開始
	    work->se_time -= 65;
	}
    }

    work->se_time += 5;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void moven_getpos						*/
/*	引数:	Work	*work							*/
/*	説明:	動かした挙げ句に位置を取得					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void moven_getpos(FVECTOR* pos, float vx, float vy, float vz){
    FVECTOR	tempvec;
    FMATRIX	tempmat;

    tempvec.vx = vx;
    tempvec.vy = vy;
    tempvec.vz = vz;
    DG_MovePos(&tempvec);
    DG_GetPos(&tempmat) ;
    _sceVu0CopyVector(pos, (FVECTOR*)tempmat.m[3] );
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_effect						*/
/*	引数:	Work	*work							*/
/*	説明:	エフェクトの位置を動かす					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void move_effect(Work *work){

    Har_FlareMove(work);	// *******フレアの挙動

    {
	DG_SetPos( &work->body.objs->objs[0].world ) ;
	moven_getpos(&work->dam_smk_pos[0], 300.0f, 1000.0f, -2000.0f);
	DG_SetPos( &work->body.objs->objs[0].world ) ;
	moven_getpos(&work->dam_smk_pos[1], 3300.0f, 0.0f, -500.0f);

	DG_SetPos( &work->body.objs->objs[0].world ) ;	//尾翼
	moven_getpos(&work->dam_smk_pos[4], 300.0f, 0.0f, -5000.0f);

	DG_SetPos( &work->body.objs->objs[0].world ) ;	//翼つけね
	moven_getpos(&work->dam_smk_pos[2], -1100.0f, 1000.0f, 1500.0f);

	DG_SetPos( &work->body.objs->objs[0].world );	//インテーク横
	moven_getpos(&work->dam_smk_pos[3], 900.0f, 200.0f, 4000.0f);
    }

    
    {
	SVECTOR	rot	= {2048, -50, 0, 0};
	FVECTOR	shift	= {1010.0f, 0.0f, 490.0f, 0.0f};
	
	//前ノズル左
	DG_SetPos( &work->body.objs->objs[10].world ) ;
	DG_RotatePos( &rot ) ;
	DG_MovePos( &shift );
	DG_GetPos( &work->nozel[0]  ) ;
	
	//前ノズル右
	rot.vy = rot.vy*(-1);
	shift.vx = shift.vx * (-1.0f); 
	DG_SetPos( &work->body.objs->objs[10].world ) ;
	DG_RotatePos( &rot ) ;
	DG_MovePos( &shift );
	DG_GetPos( &work->nozel[1]  ) ;
    }
    
    {
	SVECTOR	rot	= {2048, -70, 0, 0};
	FVECTOR	shift	= {800.0f, 0.0f, 320.0f, 0.0f};
	

	// ****ブラーの ON／OFF
	if (work->body.objs->flag & DG_FLAG_INVISIBLE){
	    work->nozel[2].m[3][0] = work->nozel[2].m[3][2] = 
	    work->nozel[3].m[3][0] = work->nozel[3].m[3][2] = 1000000.0f;
	}
	else {
	//後ノズル左
	    DG_SetPos( &work->body.objs->objs[1].world ) ;
	    DG_RotatePos( &rot ) ;
	    DG_MovePos( &shift );
	    DG_GetPos( &work->nozel[2]  ) ;
	    
	    //後ノズル右
	    rot.vy = rot.vy*(-1);
	    shift.vx = shift.vx * (-1.0f); 
	    DG_SetPos( &work->body.objs->objs[1].world ) ;
	    DG_RotatePos( &rot ) ;
	    DG_MovePos( &shift );
	    DG_GetPos( &work->nozel[3]  ) ;
	}
    }
    
    //鼻尻尾
    {
	FVECTOR	shift1	= {0.0f, 100.0f, 7700.0f, 0.0f};
	FVECTOR	shift2	= {0.0f, 400.0f, -6500.0f, 0.0f};

	FMATRIX	tmpmat;
	
	DG_SetPos( &work->body.objs->objs[0].world ) ;
	DG_MovePos( &shift1 );
	DG_GetPos( &tmpmat ) ;
	Fvec_from_Matrix(&work->pos_hanna, &tmpmat);
	
	DG_SetPos( &work->body.objs->objs[0].world ) ;
	DG_MovePos( &shift2 );
	DG_GetPos( &tmpmat ) ;
	Fvec_from_Matrix(&work->pos_sippo, &tmpmat);
    }


    //筋雲
    {

	FVECTOR	shift1	= {4477.0f, -157.70f, -1811.76f, 0.0f};
	FVECTOR	shift2	= {-4477.0f, -157.70f, -1811.76f, 0.0f};
	FMATRIX	tmpmat;
	
	DG_SetPos( &work->body.objs->objs[0].world ) ;
	DG_MovePos( &shift1 );
	DG_GetPos( &tmpmat ) ;
	Fvec_from_Matrix(&work->wing_edge1, &tmpmat);
	
	DG_SetPos( &work->body.objs->objs[0].world ) ;
	DG_MovePos( &shift2 );
	DG_GetPos( &tmpmat ) ;
	Fvec_from_Matrix(&work->wing_edge2, &tmpmat);

	work->wsmoke_width = 2000.0f*work->dist_new/550000.0f;
	if (work->wsmoke_width > 2000.0f){
	    work->wsmoke_width = 2000.0f;
	}
	if (work->wsmoke_width < 200.0f){
	    work->wsmoke_width = 200.0f;
	}


	// ****雲の ON／OFF
	if (work->body.objs->flag & DG_FLAG_INVISIBLE){
	    work->wing_smoke_flg = OFF;
	}
	else if (work->wing_smoke_flg == SET_FLG_OFF){
	    work->wing_smoke_flg = OFF;
	}
	else if ((int)_FVecLen3( &work->Speed) > 2000){
	    work->wing_smoke_flg = ON;
	}
	else {
	    work->wing_smoke_flg = OFF;
	}
    }
    
    //機銃
    {
	//	FVECTOR	shift	= {500.0f, -600.0f, 1750.0f, 0.0f};
	FVECTOR	shift	= {500.0f, -650.0f, 1800.0f, 0.0f};
	FMATRIX	tmpmat;
	FMATRIX	gunmat;
	
	DG_SetPos( &work->body.objs->objs[0].world ) ;
	DG_MovePos( &shift );
	DG_GetPos( &tmpmat ) ;
	_sceVu0CopyMatrix(&gunmat, &tmpmat) ;
	Fvec_from_Matrix(&work->gun_pos, &tmpmat);
	
	_sceVu0CopyMatrix(&tmpmat, &work->body.objs->objs[0].world ) ;
	
	tmpmat.m[ 3 ][ 0 ] = 0.0f;
	tmpmat.m[ 3 ][ 1 ] = 0.0f;
	tmpmat.m[ 3 ][ 2 ] = 0.0f;
	shift.vx = 0.0f;
	shift.vy = 0.0f;
	shift.vz = 1000.0f;
	
	DG_SetPos(&tmpmat);
	DG_MovePos(&shift);
	DG_GetPos(&tmpmat);
	Fvec_from_Matrix(&work->gun_vec, &tmpmat);
	_sceVu0Normalize(&work->gun_vec, &work->gun_vec);


	if (work->vul_fire_flg&GUN_FIRE_FLG_MODCHECK)
   {
      const int GUN_FIRE_INTERVAL = BP_IsPAL() ? 18 : 26;   //BP_FRAME_RATE - dynamically handle PAL

      if (work->gun_time_first == 0)
      {
         GM_SeSetMode (SD_E_GUNST01, &work->gun_pos, GM_SEMODE_BOMB);//SE機銃
         //printf ("start\n");
      }
	   else if ((work->gun_time%12 == 0) && (work->gun_time_first >= GUN_FIRE_INTERVAL))   //BP_FRAME_RATE - dynamically handle PAL
      {
		   GM_SeSetMode (SD_E_GUNLP02, &work->gun_pos, GM_SEMODE_BOMB);//SE機銃
		   if (work->gun_time%75 == 0)
         {	//光とSEの最初公倍数
		      work->gun_time = 0;
		   }
      }
      work->gun_time ++;	//機銃タイマ
      work->gun_time_first ++;	//機銃タイマ

	   if (work->gun_time_first == GUN_FIRE_INTERVAL)
      {
		   work->gun_time = 15;
      }

	    if (work->gun_time%5 < 2){//点光源
		int rgba;
	FVECTOR	shift	= {0.0f, -1000.0f, 700.0f, 0.0f};
	FMATRIX	tmpmat;
	FMATRIX	gunmat;
	FVECTOR	tmppos;
	
	DG_SetPos( &work->body.objs->objs[0].world ) ;
	DG_MovePos( &shift );
	DG_GetPos( &tmpmat ) ;
	_sceVu0CopyMatrix(&gunmat, &tmpmat) ;
	Fvec_from_Matrix(&tmppos, &tmpmat);

		

		SetRGBA_Char(work->temp_light_col, &rgba);
		DG_SetTmpLight2 (
		    &tmppos,
		    1500.0f,
		    3000.0f,
		    rgba,
		    LIT_FLAG_CHARAONLY );
	    }
	    
	    if ((work->gun_time%5 == 0)|(work->gun_time%5 == 2)|(work->gun_time%5 == 4)){ //弾発射
		NewHarGunBulet( &gunmat, (int)_FVecLen3( &work->control.step)*1.0f , &work->vul_fire_flg, &work->vul_hit_flg);
	    }
	}
	else {	// とってつけた
	    work->gun_time_first = 0;
	}	
	
    }
    //ライト
    {
	FVECTOR	shift1	= {170.0f, 1130.0f, -90.0f, 0.0f};
	FVECTOR	shift2	= {0.0f, -550.0f, -1700.0f, 0.0f};
	FMATRIX	tmpmat;

	// ****ライトの ON／OFF
	if (work->body.objs->flag & DG_FLAG_INVISIBLE){
	    work->body_light_pos1.vx = work->body_light_pos1.vz = 
	    work->body_light_pos2.vx = work->body_light_pos2.vz = 1000000.0f;
	}
	else {
	    DG_SetPos( &work->body.objs->objs[0].world ) ;
	    DG_MovePos( &shift1 );
	    DG_GetPos( &tmpmat ) ;
	    Fvec_from_Matrix(&work->body_light_pos1, &tmpmat);
	    
	    DG_SetPos( &work->body.objs->objs[0].world ) ;
	    DG_MovePos( &shift2 );
	    DG_GetPos( &tmpmat ) ;
	    Fvec_from_Matrix(&work->body_light_pos2, &tmpmat);
	}
    }
    //びっくり
    {
	FVECTOR	shift = {0.0f, 1700.0f, 5000.0f, 0.0f};
	DG_SetPos( &work->body.objs->objs[0].world ) ;
	DG_MovePos( &shift );
	DG_GetPos( &work->headmark_pos);
    }

}

/*******************************<Global function>********************************/
/*	名前:	void Har_SeEffect						*/
/*	引数:	Work	*work							*/
/*	説明:	エフェクトの更新 ＳＥ呼びだし					*/
/********************************************************************************/
void Har_SeEffect(Work *work)
{
    move_effect(work);
}

extern void *NewDemoHarEffect( int name, OBJECT *objs);
/*******************************<Global function>********************************/
/*	名前:	void Har_SetupEffect						*/
/*	引数:	Work	*work							*/
/*	説明:	エフェクトの設定 GetResource内で呼びだし			*/
/********************************************************************************/
void Har_SetupEffect(Work *work)
{
    short loop;
    for (loop=0; loop<5; loop++){
	work->dam_smk_flg[loop] = ON;
    }

    {	//	*****排気の熱でゆらゆら*****
	extern void *NewSmokeBlurEffect( FMATRIX *world, 
					 int start_speed, int end_speed,  int start_size, int end_size,
					 int spot_size,   int spot_angle, int n_prims,    int interval,
					 int color, int flag );
	int	rgba;


	SetRGBA_Char(work->nozl_blur_col, &rgba);
	
	//後ノズル左
	GV_SetActorChild( work,
			  NewSmokeBlurEffect( &work->nozel[2],
					      230, 200, 800, 1000,
					      200, 110, 30, 1,
					      rgba, 0));
	//後ノズル右
	GV_SetActorChild( work,
			  NewSmokeBlurEffect( &work->nozel[3],
					      230, 200, 800, 1000,
					      200, 110, 30, 1,
					      rgba, 0 ));
    }
    
    
    {	//	******動作ライト*****
	int	rgba;
	SetRGBA_CharR(work->bonbori_col, &rgba);
	GV_SetActorChild( work, NewHarrierLight( &work->body_light_pos1, rgba )) ;
	GV_SetActorChild( work, NewHarrierLight( &work->body_light_pos2, rgba )) ;
    }
    
    
    {	//	******筋雲*****
	GV_SetActorChild( work,
			  NewHarrierWingSmoke( &work->wing_edge1, &work->wsmoke_width, &work->wing_smoke_flg) );
	GV_SetActorChild( work,
			  NewHarrierWingSmoke( &work->wing_edge2, &work->wsmoke_width, &work->wing_smoke_flg) );
    }
    
    {	//	******バルカン*****
	GV_SetActorChild( work,
			  NewHarrierVulcanFire(&work->gun_pos, &work->gun_vec, &work->vul_fire_flg) );
    }

    {	//	*****風エフェクト*****
	GV_SetActorChild( work,
			  NewWaterWindSplush(
			      &work->body.objs->objs[1].world,
			      20000.0f, 20000.0f, NULL )
	    );//子アクターを呼ぶ
    }


    {	//	*****バーニアエフェクト*****
	int rgba0, rgba1;


	SetRGBA_CharR(work->nozl_atack_col, &rgba0);
	SetRGBA_CharR(&work->nozl_atack_col[4], &rgba1);

	NewHarrierVernier( (FVECTOR*)(work->nozel[2].m[3]), &work->burn_dir[0], &work->burn_flag, rgba0, rgba1);
	NewHarrierVernier( (FVECTOR*)(work->nozel[3].m[3]), &work->burn_dir[1], &work->burn_flag, rgba0, rgba1);

    }

}





