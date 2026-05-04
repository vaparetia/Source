/*
    kmsound.c
    かもめ音関連
    2001/05/08	Masafumi Okuta
    $Id: kmsound.c,v 1.1.1.3 2002/11/19 11:48:04 Yoshizawa1 Exp $
*/

enum{ // 音
KMM_SE_FLY01, 		// 海鳥羽ばたき１
KMM_SE_FLY02, 		// 海鳥羽ばたき２
KMM_SE_FLY03, 		// 海鳥羽ばたき３
KMM_SE_FLY04, 		// 海鳥羽ばたき４
KMM_SE_FLY11, 		// 海鳥羽ばたき５
KMM_SE_FLY12, 		// 海鳥羽ばたき６
KMM_SE_FLY13, 		// 海鳥羽ばたき７
KMM_SE_FLY14, 		// 海鳥羽ばたき８
KMM_SE_SOR01, 		// 海鳥飛び立ち羽ばたき１←
KMM_SE_SOR02, 		// 海鳥飛び立ち羽ばたき２←コレらをヒッチコック攻撃の時にも使うか
KMM_SE_ATK01, 		// 海鳥くちばし攻撃ヒット
KMM_SE_HIT01, 		// 海鳥フン、プレイヤーに当たる
KMM_SE_OTI01, 		// 海鳥フン、床に落ちる１
KMM_SE_OTI02, 		// 海鳥フン、床に落ちる２
KMM_SE_VOX01, 		// 海鳥鳴き声１
KMM_SE_VOX02, 		// 海鳥鳴き声２
KMM_SE_VOX03, 		// 海鳥鳴き声３
KMM_SE_VOX04, 		// 海鳥鳴き声４
KMM_SE_DMG01, 		// 海鳥ダメージ鳴き声１   ok
KMM_SE_DMG02, 		// 海鳥ダメージ鳴き声２   ok
KMM_SE_OUT01, 		// 海鳥アウト鳴き声１	ok
KMM_SE_OUT02, 		// 海鳥アウト鳴き声２	ok
KMM_SE_KMODWN01,	// カモメダウン
KMM_SE_KMOFOOTL,	// カモメ足音（左）
KMM_SE_KMOFOOTR,	// カモメ足音（右）
KMM_SE_KMOTUTUK,	// カモメ床ついばみ
KMM_SE_MAX,
};

static int KAMOME_SE_TBL[KMM_SE_MAX] = {
SD_A_KMOFLY01, // 海鳥羽ばたき１
SD_A_KMOFLY02, // 海鳥羽ばたき２
SD_A_KMOFLY03, // 海鳥羽ばたき３
SD_A_KMOFLY04, // 海鳥羽ばたき４
SD_A_KMOFLY11, // 海鳥羽ばたき５
SD_A_KMOFLY12, // 海鳥羽ばたき６
SD_A_KMOFLY13, // 海鳥羽ばたき７
SD_A_KMOFLY14, // 海鳥羽ばたき８
SD_A_KMOSOR01, // 海鳥飛び立ち羽ばたき１←
SD_A_KMOSOR02, // 海鳥飛び立ち羽ばたき２←コレらをヒッチコック攻撃の時にも使うか
SD_A_KMOATK01, // 海鳥くちばし攻撃ヒット
SD_A_FUNHIT01, // 海鳥フン、プレイヤーに当たる
SD_A_FUNOTI01, // 海鳥フン、床に落ちる１
SD_A_FUNOTI02, // 海鳥フン、床に落ちる２
SD_A_KMOVOX01, // 海鳥鳴き声１
SD_A_KMOVOX02, // 海鳥鳴き声２
SD_A_KMOVOX03, // 海鳥鳴き声３
SD_A_KMOVOX04, // 海鳥鳴き声４
SD_A_KMODMG01, // 海鳥ダメージ鳴き声１
SD_A_KMODMG02, // 海鳥ダメージ鳴き声２
SD_A_KMOOUT01, // 海鳥アウト鳴き声１
SD_A_KMOOUT02, // 海鳥アウト鳴き声２
SD_A_KMODWN01, // カモメダウン
SD_A_KMOFOOTL, // カモメ足音（左）
SD_A_KMOFOOTR, // カモメ足音（右）
SD_A_KMOTUTUK, // カモメ床ついばみ
};

// SE再生:かもめ個体用
static int KAMOME_CallSe( KAMOME*  kamome, 	// かもめワーク
			  int      nSeIndex,	// SE番号（インデックス）
			  FVECTOR* pvecPos,	// SE発生位置
			  int      nMode)	// SE再生モード
{
    GM_SeSetMode( KAMOME_SE_TBL[nSeIndex], pvecPos, nMode);	// 効果音コール
    return 1;
}
#if 0
// SE再生:かもめマネージャ用
static int KMMNG_CallSe( KAMOME_MNG*    kmmng, 		// かもめマネージャワーク
			 int      	nSeIndex,	// SE番号（インデックス）
			 FVECTOR* 	pvecPos,	// SE発生位置
			 int      	nMode)		// SE再生モード
{
    GM_SeSetMode( KAMOME_SE_TBL[nSeIndex], pvecPos, nMode);	// 効果音コール
    return 1;
}
#endif
// かもめ発声
static int KAMOME_Say( KAMOME*  kamome, 	// かもめワーク
		       int      nSeIndex,	// SE番号（インデックス）
		       int      nMode)		// SE再生モード
{
    return KAMOME_CallSe( kamome, nSeIndex, (FVECTOR *)&BODYWORLD( &kamome->body, KAMOME16_ATAMA ).m[3][0], nMode );
}

// かもめ音用リストクリア
static void KMM_ClearSeList( KAMOME_MNG* kmmng)
{	
    int i;

    // 攻撃音をならすカモメリスト
    for ( i = 0; i < KMM_SE_ATTACK_NUM;	i++){
	kmmng->pkmmAtcSe[i] = NULL;
    }
    // 接近カモメリスト
    for ( i = 0; i < KMM_SE_NEAR_NUM; i++){
	kmmng->pkmmNearSe[i] = NULL;
    }

}
// 攻撃音リスト
static int KMM_AddAtcSeList( KAMOME_MNG* kmmng, KAMOME* kamome)
{
    int i;

    if ( kmmng == NULL ) return 0;

    for ( i = 0; i < KMM_SE_ATTACK_NUM;	i++){
	if ( kmmng->pkmmAtcSe[i] == NULL ){ // 空なら
	    kmmng->pkmmAtcSe[i] = kamome;
	    return 1;
	}
    }
    return 0;
}
// 接近リスト
static int KMM_AddNearSeList( KAMOME_MNG* kmmng, KAMOME* kamome)
{
    int i;

    if ( kmmng == NULL ) return 0;

    for ( i = 0; i < KMM_SE_NEAR_NUM;	i++){
	if ( kmmng->pkmmNearSe[i] == NULL ){ // 空なら
	    kmmng->pkmmNearSe[i] = kamome;
	    return 1;
	}
    }
    return 0;
}



// かもめ音管理ワーク初期化
static void KMM_InitSoundWork( KAMOME* kamome)
{
    int i;
    KAMOME_MNG* kmmng;
    kmmng = (KAMOME_MNG*)kamome->pvManager;
    kamome->nAtcSeCntr  = KMM_GetDemoRandom( kmmng, 0, 40);
    kamome->nWingSeCntr = KMM_GetDemoRandom( kmmng, 20, 40);
    kamome->nSayCntr 	= KMM_GetDemoRandom( kmmng, 60, 90);

    for ( i = 0; i < KMM_SE_NEAR_NUM; i++)
	kmmng->nWingSeOffset[i] = BP_PS2_rand() % 4;
}
// かもめ音管理ワーク更新
static void KMM_UpdateSoundWork( KAMOME* kamome)
{
    if (kamome->nAtcSeCntr > 0)		kamome->nAtcSeCntr--;
    if (kamome->nWingSeCntr > 0)    	kamome->nWingSeCntr--;
    if (kamome->nSayCntr > 0)    	kamome->nSayCntr--;
}

// かもめモーション依存音鳴らし
static void KMM_CallMotionSe( KAMOME* kamome)
{
    KMMACT* 	act;
    KAMOME_MNG* kmmng;

    kmmng = (KAMOME_MNG*)kamome->pvManager;
    act = &kamome->action;

    switch ( act->current_mot ){ 
    case KMM_MOT_FLYAWAY:	// 飛び去り
	if ( kamome->mmt_ctrl.m_time == 48){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_FLIP);
	}
	break;
    case KMM_MOT_LANDING:
	if ( kamome->mmt_ctrl.m_time == 4 ||
	     kamome->mmt_ctrl.m_time == 24 ||
	     kamome->mmt_ctrl.m_time == 45 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_FLIP);
	}
	if ( kamome->mmt_ctrl.m_time == 62 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_FLIP);
	}
	break;
    case KMM_MOT_FLYING:
	if ( kamome->mmt_ctrl.m_time == 5 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_WEAK);
	}
	break;
#if 0
    case KMM_MOT_FLYING02:
	if ( kamome->mmt_ctrl.m_time == 4 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_WEAK);
	}
	break;
#endif
    case KMM_MOT_FLYING03:
	if ( kamome->mmt_ctrl.m_time == 11 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_WEAK);
	}
	break;
#if 0
    case KMM_MOT_WALK_A:
	if ( kamome->mmt_ctrl.m_time == 2 || kamome->mmt_ctrl.m_time == 32 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WALK);
	}
	break;
#endif
    case KMM_MOT_WALK_B:
	if ( kamome->mmt_ctrl.m_time == 2 || kamome->mmt_ctrl.m_time == 16 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WALK);
	}
	break;
    case KMM_MOT_WALK_END:
	if ( kamome->mmt_ctrl.m_time == 13 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WALK);
	}
	break;
#if 0
    case KMM_MOT_PICK_A:
	if ( kamome->mmt_ctrl.m_time == 19 ||
	     kamome->mmt_ctrl.m_time == 34 ||
	     kamome->mmt_ctrl.m_time == 46 ||
	     kamome->mmt_ctrl.m_time == 54 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_PICK);
	}
	break;
#endif
    case KMM_MOT_PICK_B:
	if ( kamome->mmt_ctrl.m_time == 15 ||
	     kamome->mmt_ctrl.m_time == 27 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_PICK);
	}
	break;
    case KMM_MOT_SURPRISE:
	if ( kamome->mmt_ctrl.m_time == 11 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_FLIP);
	}
	break;
    case KMM_MOT_WAKE_LOW:
	if ( kamome->mmt_ctrl.m_time == 66 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_WEAK);
	}
	break;
    case KMM_MOT_DAM1_START:
	if ( kamome->mmt_ctrl.m_time == 1 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_STRG);
	}
	break;
    case KMM_MOT_DAM1_END:
	if ( kamome->mmt_ctrl.m_time == 6 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_DOWN);
	}
	break;
#if 0
    case KMM_MOT_DAM2_START:
	if ( kamome->mmt_ctrl.m_time == 9 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_WING_STRG);
	}
	break;
    case KMM_MOT_DAM2_END:
	if ( kamome->mmt_ctrl.m_time == 6 ){
	    KMM_SetFlag( kamome, KMM_FLAG_SE_DOWN);
	}
	break;
#endif
    }	

}




