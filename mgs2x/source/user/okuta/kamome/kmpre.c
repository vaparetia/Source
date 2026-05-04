/*
    kmpre.c
    カモメ前処理
    2001/05/02 Masafumi Okuta
    $Id: kmpre.c,v 1.1.1.3 2002/11/19 11:48:04 Yoshizawa1 Exp $
*/
// 前処理
static void PreProcess( KAMOME* kamome)
{
    KMMACT* act;

    act = &kamome->action;

    // マトリクス設定
    DG_SetPos2( &kamome->mov, &kamome->rot);
    DG_GetPos( &kamome->matOrg);

    act->dir     = -1;
    act->pad     = 0;
//    kamome->hzx_id = GM_GetHzxGroupID( kamome->map ); 

    KMM_RecieveMessage( kamome );	// メッセージ受信

    KMM_SearchAim( kamome, kamome->nAimName);

    // ヘッドマークチェック
    if ( kamome->think.nThink3 < TH3_CHECK_FEINT_START || kamome->think.nThink3 > TH3_CHECK_SLEEP_END){
	KMM_EraseHeadMark( kamome);
    }

    // ダメージ関数チェック
    if ( kamome->think.nThink3 >= TH3_CHECK_FEINT_START && kamome->think.nThink3 <= TH3_CHECK_WAKEUP_END ){ // ダウン＆スリープ
	KMM_SetCheckDamage( kamome, KMM_CheckDownDamage); 
    }else{
	KMM_SetCheckDamage( kamome, KMM_CheckDamage);
    }

    // LOD
    kamome->nLod = KMM_LOD_NONE;

}
// 後処理
static void AfterProcess( KAMOME* kamome)
{
    GM_MoveTargetMap( &kamome->trg, &kamome->mov, kamome->map ) ;
    // モーション再生依存の音鳴らし
    KMM_CallMotionSe( kamome );
    // 音関連更新
    KMM_UpdateSoundWork( kamome);

}



