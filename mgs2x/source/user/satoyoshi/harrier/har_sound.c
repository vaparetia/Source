/********************************************************************************/
/*	Har_sound.c								*/
/*	ハリア音関連								*/
/*	2001/07/09 H.Satoyoshi							*/
/*	$Id: har_sound.c,v 1.1.1.3 2002/11/19 11:48:25 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#if 0
// ハリア戦 里吉管理 w25a
//ライデン
vc104501	//プリスキン！！		00
vc104502	//まさか！？			01
vc104503	//なに！？			02
//ステージ破壊攻撃前
vc104510	//これはどうだ？		03
vc104511	//そろそろ終りにするか？	04
//ステージ破壊後音声
vc104512	//ほぅ！やるな！		05
vc104513	//しぶとい奴だ！		06
//ライフわずか
vc104514	//くそっ			07
vc104515	//やるなっ！			08
vc104516	//はぁはぁ			09
//ソリダスカサッカにちょっかい
vc104524	//邪魔だ			10
vc104525	//どけ！兄弟！			11
vc104526	//うるさい！蝿が		12
//挑発
vc104530	//そんなものか！		13
vc104531	//どうした！？			14
vc104532	//ははははは			15
vc104509	//死ぃねぇ			16
#endif

/********************************************************************************/
/*	Program									*/
/********************************************************************************/


#define MAX_DIS_HARRIER	(1200000)

/* タービン */
#define HARRIER_TARBIN	0x10

/* ホバリング */
#define HARRIER_HOBARING	0x11

/* ドップラー */
#define HARRIER_DOPPLER	0x12

/* カサッカ 羽音 */
#define KASACKA_WING	0x13

/* カサッカ タービン */
#define KASACKA_TARBIN	0x14


#define	TAR_DIS  3000000/100
#define	JET_DIS  3000000/100
#define	FRAN_DIS 3000000/100
#define	TAR_VOL	 63/100

#if 0
static VOLUMECURVE jet_ownview = {
    2,
    SE_COS_60,
    SE_COS_90,
    {6*JET_DIS, 18*JET_DIS, -1, -1},
    {78*TAR_VOL, 20*TAR_VOL, 0, 0, 0}, 
    {6*JET_DIS, 18*JET_DIS, -1, -1},
    {78*TAR_VOL, 20*TAR_VOL, 0, 0, 0}, 
    1.0f
};


static VOLUMECURVE tarbin_ownview = {
    4,
    SE_COS_60,
    SE_COS_90,
    {2*TAR_DIS, 4*TAR_DIS, 36*TAR_DIS, 68*TAR_DIS},
    {78*TAR_VOL, 78*TAR_VOL, 23*TAR_VOL, 7*TAR_VOL, 0}, 
    {2*TAR_DIS, 4*TAR_DIS, 36*TAR_DIS, 68*TAR_DIS},
    {78*TAR_VOL, 78*TAR_VOL, 23*TAR_VOL, 7*TAR_VOL, 0}, 
    1.0f
};

#endif


static VOLUMECURVE tarbin_ownview = {
    4,
    SE_COS_60,
    SE_COS_90,
    {2*TAR_DIS, 4*TAR_DIS, 36*TAR_DIS, 75*TAR_DIS},
    {78*TAR_VOL, 78*TAR_VOL, 23*TAR_VOL, 7*TAR_VOL, 0}, 
    {2*TAR_DIS, 4*TAR_DIS, 36*TAR_DIS, 68*TAR_DIS},
    {78*TAR_VOL, 78*TAR_VOL, 23*TAR_VOL, 7*TAR_VOL, 0}, 
    1.0f
};



static VOLUMECURVE jet_ownview = {
    3,
    SE_COS_60,
    SE_COS_90,
    {3*JET_DIS, 18*JET_DIS, 27*JET_DIS, -1},
    {78*TAR_VOL, 78*TAR_VOL, 10*TAR_VOL, 0, 0}, 
    {4*JET_DIS, 16*JET_DIS, 28*JET_DIS, -1},
    {78*TAR_VOL, 78*TAR_VOL, 10*TAR_VOL, 0, 0}, 
    1.0f
};


static VOLUMECURVE fran_ownview = {
    4,
    SE_COS_60,
    SE_COS_90,
    {25*FRAN_DIS, 50*FRAN_DIS, 75*FRAN_DIS, 100*FRAN_DIS},
    {19*TAR_VOL, 35*TAR_VOL, 23*TAR_VOL, 11*TAR_VOL, 0*TAR_VOL}, 
    {25*FRAN_DIS, 50*FRAN_DIS, 75*FRAN_DIS, 100*FRAN_DIS},
    {19*TAR_VOL, 35*TAR_VOL, 23*TAR_VOL, 11*TAR_VOL, 0*TAR_VOL}, 
    1.0f
};


static VOLCURVES har_tarbin_curves = {
    &tarbin_ownview,
    &tarbin_ownview,
    &tarbin_ownview,
    &tarbin_ownview
};

static VOLCURVES har_jet_curves = {
    &jet_ownview,
    &jet_ownview,
    &jet_ownview,
    &jet_ownview
};

static VOLCURVES har_fran_curves = {
    &fran_ownview,
    &fran_ownview,
    &fran_ownview,
    &fran_ownview
};






/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void sound_control						*/
/*	引数:	Work	*work							*/
/*	説明:	サウンドコントロール						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifdef PSX2
static void HarrierBGMFader( Work *work )
{
    FVECTOR	shift;
    float	distance;
    int		freq;
    int		volume = 0x2f;
    int		pan = 0x20;
    float bp_angle = 0.f;

    // 前フレームの距離を保存
    work->dist_old = work->dist_new;


    //	距離の計算
    SAT_Minus_FVECTOR(&shift, &GM_PlayerFindPos, &work->control.mov);

    //  前ターンとの距離の差を計算
    work->dist_new = (int)_FVecLen3( &shift);
    distance = work->dist_old - work->dist_new;

    //    printf ("%f  %f\n", distance, work->dist_new);

    // その差からドップラー効果(ピッチの値)を計算
    if (distance > 0){
	freq = 0x40 + (int)distance*40/4000;
	if (freq > 0x54){
	    freq = 0x54;
	}
    }
    else {
	freq = 0x40 + (int)distance*40/4000;
	if (freq < 0x2c){
	    freq = 0x2c;
	}
    }

    work->distance = distance;

    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &har_tarbin_curves, &bp_angle);


   if ( BP_IsPAL()==FALSE )
   {
       //   **********ハリア交錯音
       if ( (work->distance>2000.0f) && (distance>2000.0f) &&
	    (57000.0f<work->dist_new) && (work->dist_new < 60000.0f) ){

	   if (pan >= 0x30){
	       GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWINGL);
	   }
	   if (pan <= 0x10){
	       GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWINGR);
	   }
	   else {
	       printf("##############CENTER\n");
	       GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWING1);
	   }
       }
   }
   else
   {
       //   **********ハリア交錯音
       if ( (work->distance>(2000.0f*5/6)) && (distance>(2000.0f*5/6)) &&
	    (57000.0f<work->dist_new) && (work->dist_new < 60000.0f) ){

	   if (pan >= 0x30){
	       GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWINGL);
	   }
	   if (pan <= 0x10){
	       GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWINGR);
	   }
	   else {
	       printf("##############CENTER\n");
	       GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWING1);
	   }
       }
   }

    if (work->pan_hokan == ON)
    {
	FVECTOR	sabun;
	float	dist;
	_sceVu0SubVector(&sabun, &GM_PlayerFindPos, &work->control.mov);
	dist = _FVecLen2(&sabun);
	if (dist < 16000.0f){
	    pan = (int)(((0x20*(16000.0f-dist))+(pan*dist))/16000.0f);
	    //	    printf ("Near %f %x\n", dist, pan);
	    //	    printf ("*");
	}
    }
    //    printf ("%x %x %x  ",freq ,pan, volume);

    GM_MixConvFrequencyFader(HARRIER_TARBIN, freq, pan, volume);


    if (distance > 0){
	freq = 0x40 + (int)distance*40/2000;
	if (freq > 0x68){
	    freq = 0x68;
	}
    }
    else {
	freq = 0x40 + (int)distance*40/2000;
	if (freq < 0x18){
	    freq = 0x18;
	}
    }


    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &har_jet_curves, &bp_angle);
    if (work->pan_hokan == ON)
    {
	FVECTOR	sabun;
	float	dist;
	_sceVu0SubVector(&sabun, &GM_PlayerFindPos, &work->control.mov);
	dist = _FVecLen2(&sabun);
	if (dist < 16000.0f){
	    pan = (int)(((0x20*(16000.0f-dist))+(pan*dist))/16000.0f);
	}
    }
    //    printf ("%x %x %x  ",freq ,pan, volume);

    GM_MixConvFrequencyFader(HARRIER_HOBARING, freq, pan, volume);

    freq = 0x18 + (int)(fabs(work->dist_new)/2000.0f);
    
    if (freq > 0x68){
    	freq = 0x68;
    }

    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &har_fran_curves, &bp_angle);
    if (work->pan_hokan == ON)
    {
	FVECTOR	sabun;
	float	dist;
	_sceVu0SubVector(&sabun, &GM_PlayerFindPos, &work->control.mov);
	dist = _FVecLen2(&sabun);
	if (dist < 16000.0f){
	    pan = (int)(((0x20*(16000.0f-dist))+(pan*dist))/16000.0f);
	}
    }
    work->pan_hokan = OFF;
    //    printf ("%x %x %x\n",freq ,pan, volume);

    GM_MixConvFrequencyFader(HARRIER_DOPPLER, freq, pan, volume);

    if ( 0 ) {/* 他のBGM OFF */
	int i ;
	for(i=0;i<0x10;i++){
	    GM_MixConvFader( i, GM_PAN_CENTER, 0 ) ;
	}
	    GM_MixConvFader( 13, GM_PAN_CENTER, 0 ) ;
	    GM_MixConvFader( 14, GM_PAN_CENTER, 0 ) ;
    }

}
#else	//PSX2
static void HarrierBGMFader( Work *work )
{
    FVECTOR	shift;
    float	distance;
    int		freq;
    int		volume = 0x2f;
    int		pan = 0x20;

    // 前フレームの距離を保存
    work->dist_old = work->dist_new;

    //	距離の計算
    SAT_Minus_FVECTOR(&shift, &GM_PlayerFindPos, &work->control.mov);
	
    //  前ターンとの距離の差を計算
    work->dist_new = (int)_FVecLen3( &shift);
    distance = work->dist_old - work->dist_new;

    //    printf ("%f  %f\n", distance, work->dist_new);

    // その差からドップラー効果(ピッチの値)を計算
    if (distance > 0){
		freq = 0x40 + (int)distance*40/4000;
		if (freq > 0x54){
			freq = 0x54;
		}
    }
    else {
		freq = 0x40 + (int)distance*40/4000;
		if (freq < 0x2c){
			freq = 0x2c;
		}
    }

    work->distance = distance;

    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &har_tarbin_curves);


#ifndef PAL          //BP JG - unused as it's #ifdefd out.
    //   **********ハリア交錯音
    if ( (work->distance>2000.0f) && (distance>2000.0f) &&
	 (57000.0f<work->dist_new) && (work->dist_new < 60000.0f) ){

	if (pan >= 0x30){
	    GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWINGL);
	}
	if (pan <= 0x10){
	    GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWINGR);
	}
	else {
	    printf("##############CENTER\n");
	    GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWING1);
	}
    }
#endif


#ifdef PAL
    //   **********ハリア交錯音
    if ( (work->distance>(2000.0f*5/6)) && (distance>(2000.0f*5/6)) &&
		 (57000.0f<work->dist_new) && (work->dist_new < 60000.0f) ){
		
		if (pan >= 0x30){
			GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWINGL);
		}
		if (pan <= 0x10){
			GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWINGR);
		}
		else {
			printf("##############CENTER\n");
			GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWING1);
		}
    }
#endif

    if (work->pan_hokan == ON)
    {
		FVECTOR	sabun;
		float	dist;
		_sceVu0SubVector(&sabun, &GM_PlayerFindPos, &work->control.mov);
		dist = _FVecLen2(&sabun);
		if (dist < 16000.0f){
			pan = (int)(((0x20*(16000.0f-dist))+(pan*dist))/16000.0f);
			//	    printf ("Near %f %x\n", dist, pan);
			//	    printf ("*");
		}
    }
    //    printf ("%x %x %x  ",freq ,pan, volume);

	sd_3dsrc_setvol( work->hTarbin, GM_ConvertVol( volume ) );
	sd_3dsrc_setpitch( work->hTarbin, GM_ConvertFreq( freq ) );
	sd_3dsrc_setpos( work->hTarbin, &work->control.mov );
	
    if (distance > 0){
		freq = 0x40 + (int)distance*40/2000;
		if (freq > 0x68){
			freq = 0x68;
		}
    }
    else {
		freq = 0x40 + (int)distance*40/2000;
		if (freq < 0x18){
			freq = 0x18;
		}
    }

    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &har_jet_curves);
    if (work->pan_hokan == ON)
    {
		FVECTOR	sabun;
		float	dist;
		_sceVu0SubVector(&sabun, &GM_PlayerFindPos, &work->control.mov);
		dist = _FVecLen2(&sabun);
		if (dist < 16000.0f){
			pan = (int)(((0x20*(16000.0f-dist))+(pan*dist))/16000.0f);
		}
    }
    //    printf ("%x %x %x  ",freq ,pan, volume);

	sd_3dsrc_setvol( work->hEngine, GM_ConvertVol( volume ) );
	sd_3dsrc_setpitch( work->hEngine, GM_ConvertFreq( freq ) );
	sd_3dsrc_setpos( work->hEngine, &work->control.mov );

    freq = 0x18 + (int)(fabs(work->dist_new)/2000.0f);
    
    if (freq > 0x68){
    	freq = 0x68;
    }

    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &har_fran_curves);
    if (work->pan_hokan == ON)
    {
		FVECTOR	sabun;
		float	dist;
		_sceVu0SubVector(&sabun, &GM_PlayerFindPos, &work->control.mov);
		dist = _FVecLen2(&sabun);
		if (dist < 16000.0f){
			pan = (int)(((0x20*(16000.0f-dist))+(pan*dist))/16000.0f);
		}
    }
    work->pan_hokan = OFF;
    //    printf ("%x %x %x\n",freq ,pan, volume);

	sd_3dsrc_setvol( work->hJet, GM_ConvertVol( volume ) );
	sd_3dsrc_setpitch( work->hJet, GM_ConvertFreq( freq ) );
	sd_3dsrc_setpos( work->hJet, &work->control.mov );
}
#endif	//PSX2


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Kak_ActPlayStream						*/
/*	引数:	Work	*work							*/
/*	説明:	ハリアー ストリーム再生管理					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Har_ActPlayStream(Work *work ){
    int  vol, pan ;
    float bp_angle;
    if (work->str_hdl){	//スネーク
	GM_SeGetVolPanFromVolCurves(&work->control.mov, &vol, &pan, &kac_se_curves, &bp_angle);
//	GM_VoxStreamSetPan( work->str_hdl, 0x3f, 0x20 ) ;
	GM_VoxStreamSetParam( work->str_hdl, &work->control.mov, GM_INVALID_ADDR, 0x3f, 0x20, 0.f ) ;
	/* 再生が終っていれば ハンドラを使っていない状態に戻す*/
	if ( GM_StreamStatus( work->str_hdl ) == GM_STREAM_STATE_END ){
	    work->str_hdl = 0;
	}
    }

    if (work->sol_speak_time > 0){
	work->sol_speak_time -= TIME_BASE;
	if (work->sol_speak_time <= 0){
	    work->sol_speak_time  = 0;
	    if (work->str_num > HAR_N_STREAM){	// =================ＳＥです
		GM_JimakuSeSet( 0x20, 0x3f, work->str_num);
		if (work->str_hdl != 0){
		    GM_StreamStop( work->str_hdl );
		    work->str_hdl = 0;
		}
	    }
	    else if (work->str_hdl == 0){	//==================音声です
//		work->str_hdl = GM_VoxStream(work->str_id[work->str_num], 0);
		work->str_hdl = GM_VoxStream(work->str_id[work->str_num], GM_STREAM_FLAG_3D);
	    }
	}
    }

    // ====================================-ＳＥ鳴らし
    if (work->har_se_time > 0){
	work->har_se_time -= TIME_BASE;
	if (work->har_se_time <= 0){
	    work->har_se_time  = 0;
	    GM_JimakuSeSetMode (work->se_num, &work->control.mov, GM_SEMODE_BOMB);
	}
    }


    if (work->gun_hit_count>30){
	work->gun_hit_count -= 30;
	switch (RAND(3)){	//=====================しゃべる
	case 0:		//どうした
	    GM_JimakuSeSetMode (SD_V_SOLDM201, &work->control.mov, GM_SEMODE_BOMB);
	    break;
	case 1:		//うるさい蝿が
	    SetSolVoice( work, 12, 3);
	    break;
	case 2:		//効かんな
	    GM_JimakuSeSetMode (SD_V_SOLDM202, &work->control.mov, GM_SEMODE_BOMB);
	    break;
	}
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Kak_ActPlayStream						*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカ ストリーム再生管理					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
inline void SetSolVoice( Work *work, int num, int time){
    work->str_num = num;
    work->sol_speak_time  = time;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Kak_ActPlayStream						*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカ ストリーム再生管理					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
inline void SetHarSE( Work *work, int num, int time){
    work->se_num = num;
    work->har_se_time  = time;
}




