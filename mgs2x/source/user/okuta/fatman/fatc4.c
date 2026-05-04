/*
    fatc4.c
    ファットマン戦に使用するC4に関する処理
    2001/03/14 Masafumi Okuta
    $Id: fatc4.c,v 1.1.1.3 2002/11/19 11:47:57 Yoshizawa1 Exp $
*/

//------------------------------------------------------------------
//------------------------------------------------------------------

// 爆弾ワーク初期化
static void InitC4Work( Work* work )
{
    int 	i;
    FAT_C4_DATA* pbombData;
    pbombData = work->bombData;

    work->pvCounter	= NULL;
    work->bCountSay	= 0;
    work->bBombActive	= 0;
    work->nBombTimer 	= BOMB_TIME;
    work->nBombCntr  	= 0;	
    work->nPutIntvCntr	= 0;
    work->nPutNum	= 0;
    work->nBombNum 	= 0;
    work->nBombCountNum = 0;
    work->nFreezeNum 	= 0;
    work->nBombMax 	= MAO_Rand( 3 ) + 2;
    work->nRunningPut 	= 0;	
    work->bPutFlag	= 0;
    work->bPutIndexUsed = 0;

    work->vecLastBombPos.vx = 56500.f;
    work->vecLastBombPos.vy = 13071.f;
    work->vecLastBombPos.vz = -106000.f;
    work->vecLastBombPos.vw = 1.f;

    // C4設置データ
    work->nC4Level	= 0;
    work->nC4Pattern    = 0;
    work->nC4Index  	= 0;

    for ( i = 0; i < MAX_BOMB_NUM; i++){
	( pbombData + i )->nID 	   = -1;
	( pbombData + i )->nStatus = -2;
	( pbombData + i )->nMapID  = -1;
	( pbombData + i )->nCount  = 0;
	( pbombData + i )->nCamDist = -1;
	( pbombData + i )->nSeNum = -1;
	( pbombData + i )->nLastSeTime = DG_TickCount;
	( pbombData + i )->matPos  = DG_UnitMatrix;
	( pbombData + i )->pputData = NULL;
	( pbombData + i )->pvRadar = NULL;
    }

    work->pCurPosData = &work->datPoint[0]; 
    work->pdatGuard   = NULL;

    // ファットマンC4マネージャ起動
    PL_FatmanC4Manager();
}

// 爆弾ワーク更新
static void UpdateC4Work( Work* work )
{
    int 		i, nPrevCountNum, nBombIndexFirst;
    int 		bBombPosGet = 0;
    FAT_C4_DATA* 	pbombData = work->bombData;

    // 設置ポイントワーク更新
    for ( i = 0; i < FAT_C4_POSDATA; i++){
	work->datPoint[i].nStatus = 0; // 爆弾存在フラグをクリア
    }

    // 設置Ｃ４ワーク更新
    work->nBombNum = 0;
    work->nFreezeNum = 0;
    nPrevCountNum = work->nBombCountNum;
    work->nBombCountNum = 0;
    nBombIndexFirst = -1;
    for ( i = 0; i < MAX_BOMB_NUM; i++){
	if ( ( pbombData + i )->nID != -1 ){	// 使用しているワークを探す
	    int nRes;
	    nRes = GM_GetKaitaiC4Status( ( pbombData + i )->nID );	// 爆弾の状態を取得

	    switch ( nRes ){
	    case -1:	// 凍らされている : 登録からは外す	
		( pbombData + i )->nStatus = -1;
		if ( ( pbombData + i )->pvRadar != NULL ){ // レーダー表示
		    work->nPlDemineC4Time = 0; // 処理ブランククリア
		    BOMB_Erase( ( pbombData + i )->pvRadar, BOMB_RADAR_FEED_OUT);
		    ( pbombData + i )->pvRadar = NULL;
		}
		if (( pbombData + i )->pputData != NULL){
		    ( pbombData + i )->pputData->nStatus = 1; // 爆弾存在フラグを設定
		}
		work->nFreezeNum++;	// 凍らした数
		break;
	    case -2:	// 存在しない
		( pbombData + i )->nID 	   = -1;
		( pbombData + i )->nStatus = -2;
		( pbombData + i )->nMapID  = -1;
		( pbombData + i )->nCamDist = -1;
		( pbombData + i )->nCount  = 0;
		( pbombData + i )->matPos  = DG_UnitMatrix;
		if (( pbombData + i )->pputData != NULL){
		    ( pbombData + i )->pputData->nStatus = 0; // 爆弾存在フラグをクリア
		}
		( pbombData + i )->pputData = NULL;
		if ( ( pbombData + i )->pvRadar != NULL ){ // レーダー表示
		    BOMB_Erase( ( pbombData + i )->pvRadar, BOMB_RADAR_FEED_OUT);
		    ( pbombData + i )->pvRadar = NULL;
		}
		break;
	    default:	// 動作中
		( pbombData + i )->nStatus = nRes;	// 残り時間を代入
		( pbombData + i )->pputData->nStatus = 1; // 爆弾存在フラグを設定
		( pbombData + i )->nCamDist = KR_CameraDis( (FVECTOR*)&( pbombData + i )->matPos.m[3][0] );
		work->nBombNum++;	// 設置数	
		if (nRes < work->nBombTimer){
		    work->nBombCountNum++;	// カウントダウン中の爆弾数
		}else{
		    ( pbombData + i )->nLastSeTime = DG_TickCount; // 音鳴り封じ
		}
		nBombIndexFirst = i;
		
		// 動作中の爆弾の位置取得
		if ( !bBombPosGet ){
		    _sceVu0CopyVector( &work->vecLastBombPos, (FVECTOR*)&( pbombData + i )->matPos.m[3][0]);
		    bBombPosGet = 1;
		}
	    }
	}else{
	    ( pbombData + i )->nID     = -1;
	    ( pbombData + i )->nStatus = -2;
	    ( pbombData + i )->nMapID  = -1;
	    ( pbombData + i )->nCount  = 0;
	    ( pbombData + i )->nCamDist = -1;
	    ( pbombData + i )->nSeNum = -1;
	    ( pbombData + i )->nLastSeTime = -1;
	    ( pbombData + i )->matPos  = DG_UnitMatrix;
	    ( pbombData + i )->pputData = NULL;
	    ( pbombData + i )->pvRadar = NULL;
	}
    }
    
    // 動作中の爆弾数を表示
    NewProgSetLeftBomb( work->nBombNum);

    // ファットマン音声
    if ( (!work->bGameClear) && work->bBombActive && work->think1 == TH1_NORMAL){
	if ( work->nBombCntr == COUNT_VMODE(1200)){
	    FAT_StreamRequestJustTime( work, FAT_STRM_BOMB_20 + BP_PS2_rand() % 2); // あと２０秒
	}else if ( work->nBombCntr == COUNT_VMODE(600)){
	    FAT_StreamRequestJustTime( work, FAT_STRM_BOMB_10 + BP_PS2_rand() % 2); // あと１０秒
	}else {
	    if ( work->bCountSay ){
		if ( work->nBombCntr == COUNT_VMODE(300)){
		    if ( MAO_Rand( 2 ) == 0 ){
			FAT_StreamRequestJustTime( work, FAT_STRM_PROV_01);  // FOXHOUND
		    }else{
			FAT_StreamRequestJustTime( work, FAT_STRM_BOMB_05 + BP_PS2_rand() % 2);  // そろそろ
		    }
		}
	    }else{ 
		// カウントダウン
		if      ( work->nBombCntr == COUNT_VMODE(300)) FAT_Say( work, SE_FATDOKN5, GM_SEMODE_BOMB);
		else if ( work->nBombCntr == COUNT_VMODE(240)) FAT_Say( work, SE_FATDOKN4, GM_SEMODE_BOMB);
		else if ( work->nBombCntr == COUNT_VMODE(180)) FAT_Say( work, SE_FATDOKN3, GM_SEMODE_BOMB);
		else if ( work->nBombCntr == COUNT_VMODE(120)) FAT_Say( work, SE_FATDOKN2, GM_SEMODE_BOMB);
		else if ( work->nBombCntr == COUNT_VMODE(60))  FAT_Say( work, SE_FATDOKN1, GM_SEMODE_BOMB);
	    }
	}
    }
    if ( work->nBombCntr > 0){ // 内部カウンタ
	work->nBombCntr--;
    }

    if ( work->bBombActive && work->nBombCntr == 0 ){ // C４時間切れ爆発ゲームオーバー
	if ( !GM_IsGameOver() ){
	    FAT_CallSe(  work, SE_C4OUT08, &GM_PlayerPosition, GM_SEMODE_BOMB); // 爆発音
	    FAT_Say( work, SE_FATDOKN0, GM_SEMODE_BOMB);
	}
	{ // ゲームオーバープロシージャの呼出
	    GCL_ARGS  gcl_args;
	    int       data[3];
	    // 爆弾の位置を渡す
	    data[0] = (int)work->vecLastBombPos.vx;
	    data[1] = (int)work->vecLastBombPos.vy;
	    data[2] = (int)work->vecLastBombPos.vz;

	    gcl_args.argc = 3;
	    gcl_args.argv = data;
	    GCL_ExecProc( work->procGameOver, &gcl_args ); // ゲームオーバー
	}
	work->bBombActive = 0;
	work->bGameOver = 1; // ゲームオーバーフラグ
	return;
    }

    // 爆弾を全部解体した
    if ( work->bBombActive && work->nBombCountNum == 0 ){ // カウントダウンしている爆弾がなかった
	work->bBombActive = 0;
	// カウンタ
        if ( work->pvCounter != NULL ){
	    GV_DestroyOtherActor( work->pvCounter ); // 破棄
	}
	// 情報送信用プロシージャコール:解体メッセージ
	if ( work->procFatInfo != 0){
	    GCL_ARGS  gcl_args;
	    int       data;
	
	    data = FAT_GCLINFO_CLEAR_BOMB;
	    gcl_args.argc = 1;
	    gcl_args.argv = &data;
	    GCL_ExecProc( work->procFatInfo, &gcl_args ); // フェーズ更新
	}
    }

}

// 新しいC4を登録 
// 成功 1
// 失敗 0
static int SetNewFatmanC4( Work*		work,
			   FAT_PUTPOINT_DATA*	pput,		// 設置場所構造体へのポインタ
			   int	 		nMapID,		// 設置するマップID
			   int	 		nTime)		// 爆発までの時間
{
    int i;
    FMATRIX mat;
    FAT_C4_DATA* 	pbombData;	// 爆弾情報構造体へのポインタ
    
    pbombData = &work->bombData[0];

    work->nLastBombLife  = work->npc.action.life;
    work->nLastBombFaint = work->m9_faint;

    for ( i = 0; i < MAX_BOMB_NUM; i++){
	if ( ( pbombData + i )->nID == -1){	// 未使用のワークがあるかチェック
	    mat = DG_UnitMatrix;
	    GV_VecToMat( &pput->vecPos, &mat );
	    DG_SetPos(&mat);
	    DG_RotatePos( &pput->vecRot ) ;
	    DG_GetPos( &mat ) ;
            ( pbombData + i )->nID = NewSetKaitaiC4Prog( &mat, nMapID, nTime);
	    { // 爆発した時のプロシージャコール
		extern void PL_SetKaitaiC4BlastProc( int );
		PL_SetKaitaiC4BlastProc( work->procShootC4Over);
	    }
#ifdef DEBUG_MODE
printf("----- FAT-C4 Put!! -----\n");
printf("LV=%d PT=%d ID=%d\n", work->nC4Level, work->nC4Pattern, work->nC4Index);
printf("POS = { %d %d %d }\n", (int)mat.m[3][0], (int)mat.m[3][1], (int)mat.m[3][2] );
#endif
	    if (( pbombData + i )->nID == -1){
		return 0;	// 起動失敗
	    }
	    ( pbombData + i )->nMapID  = nMapID;
	    ( pbombData + i )->nStatus = nTime;	// 仮代入
	    ( pbombData + i )->nSeNum  = SE_C4TIME11 + 4 * (work->nPutNum % 3);
	    ( pbombData + i )->nLastSeTime = DG_TickCount;
	    ( pbombData + i )->nCount  = nTime;
	    ( pbombData + i )->matPos  = mat;
	    ( pbombData + i )->pputData = pput;

	    pput->nStatus = 1;

	    if ( pput->nTrapName != -1 ){
		( pbombData + i )->pvRadar = NewBombAreaP( pput->nTrapName, 0x10, 0x10, 0x00);	// レーダー表示
	    }
	    
	    work->bPutIndexUsed = 1;	// インデックス使用フラグをたてる
	    work->nPutNum++;	// 設置数更新
	    return 1;	// 成功
	}
    }

    for ( i = 0; i < MAX_BOMB_NUM; i++){
	if ( ( pbombData + i )->nID != -1 &&
	     ( pbombData + i )->nStatus == -1){	// 凍結しているのワークがあるかチェック
	    // 凍結しているC4を除去
	    GM_DestroyKaitaiC4( ( pbombData + i )->nID );

	    // 凍結しているC4ワーク初期化
	    mat = DG_UnitMatrix;
	    GV_VecToMat( &pput->vecPos, &mat );
	    DG_SetPos(&mat);
	    DG_RotatePos( &pput->vecRot ) ;
	    DG_GetPos( &mat ) ;
            ( pbombData + i )->nID = NewSetKaitaiC4Prog( &mat, nMapID, nTime);
	    { // 爆発した時のプロシージャコール
		extern void PL_SetKaitaiC4BlastProc( int );
		PL_SetKaitaiC4BlastProc( work->procShootC4Over);
	    }
#ifdef DEBUG_MODE
printf("----- FAT-C4 Put!! -----\n");
printf("LV=%d PT=%d ID=%d\n", work->nC4Level, work->nC4Pattern, work->nC4Index);
printf("POS = { %d %d %d }\n", (int)mat.m[3][0], (int)mat.m[3][1], (int)mat.m[3][2] );
#endif
	    if (( pbombData + i )->nID == -1){
		return 0;	// 起動失敗
	    }
	    ( pbombData + i )->nMapID  = nMapID;
	    ( pbombData + i )->nStatus = nTime;	// 仮代入
	    ( pbombData + i )->nSeNum  = SE_C4TIME11 + 4 * (work->nPutNum % 3);
	    ( pbombData + i )->nLastSeTime = DG_TickCount;
	    ( pbombData + i )->nCount  = nTime;
	    ( pbombData + i )->matPos  = mat;
	    ( pbombData + i )->pputData = pput;
	    // レーダー表示が残っていた
	    if ( ( pbombData + i )->pvRadar != NULL ){ 
		BOMB_Erase( ( pbombData + i )->pvRadar, BOMB_RADAR_FEED_OUT);
		( pbombData + i )->pvRadar = NULL;
	    }
	    // トラップ名からレーダー表示起動
	    if ( pput->nTrapName != -1 ){
		( pbombData + i )->pvRadar = NewBombAreaP( pput->nTrapName, 0x10, 0x10, 0x00);	// レーダー表示
	    }
	    work->bPutIndexUsed = 1;	// インデックス使用フラグをたてる
	    work->nPutNum++;	// 設置数更新
	    return 1;
	}
    }

    return 0;	// 空きワークがなかった
}

// 爆弾を起動する
static void FAT_StartBombCount( Work* work ) 	// ファットマンワーク
{
    int i;

    // すでに起動している
    if ( work->bBombActive )
	return; 

    // 起動
    for ( i = 0; i < MAX_BOMB_NUM; i++ ){
	if ( work->bombData[i].nID != -1 &&
	     GM_GetKaitaiC4Status( work->bombData[i].nID ) >= 0){ // 存在していて凍っていない
	    GM_StartCountDownKaitaiC4( work->bombData[i].nID );	// 爆弾カウントダウン開始
	    work->bombData[i].nLastSeTime = DG_TickCount;
	}
    }
    // 起動フラグを設定
    work->bCountSay = MAO_Rand( 2 );
    work->bBombActive = 1;

    // 内部カウンタを設定
    work->nBombCntr = work->nBombTimer;	

    // カウンタ呼び
    work->pvCounter = NewProgTimer( 0 );
    if ( work->pvCounter != NULL ){
	GV_SetActorChild( work, work->pvCounter);
    }

    // 情報送信用プロシージャコール
    if ( work->procFatInfo != 0){
	GCL_ARGS  gcl_args;
	int       data;
	
	data = FAT_GCLINFO_STARTUP_BOMB;
	gcl_args.argc = 1;
	gcl_args.argv = &data;
	GCL_ExecProc( work->procFatInfo, &gcl_args ); // フェーズ更新
    }

    // カウンタ表示設定
    NewProgSetLeftTime( work->nBombTimer );
}
// 指定された位置データへのポインタを取得
static FAT_PUTPOINT_DATA* FAT_GetPosData( Work* work, int nLevel, int nPattern, int nIndex)
{
    return ( &work->datPoint[ work->nC4IndexData[nLevel][nPattern][nIndex] ] );
}
// 現在のレベルとパターンから任意のインデックスのデータへのポインタを取得する
static inline FAT_PUTPOINT_DATA* FAT_GetCurrPosData( Work* work, int nIndex)
{
    return ( &work->datPoint[ work->nC4IndexData[work->nC4Level][work->nC4Pattern][nIndex] ] );
}
static inline FAT_PUTPOINT_DATA* FAT_GetPosPatternPtr( Work* work, int nLevel, int nPattern)
{
    return ( &work->datPoint[ work->nC4IndexData[nLevel][nPattern][0] ] );
}
static inline int FAT_GetC4PosNum( Work* work, int nLevel, int nPattern)
{
    return FAT_C4_POSNUM;
}
static int FAT_GetCurrentC4PosNum( Work* work )
{
    return FAT_C4_POSNUM;
}

// GCL上のC4関連のデータを取得
static void FAT_GetGclBombData( Work* work ) 	
{
    char* opt;
    int i, j, n;
    int nIndexNum;
    int nTypeNum;
    int nTrapNum;
    int nPointNum;
    int nAimNum;
    int nRotNum;
    int nIndexRotNum;

    FAT_PUTPOINT_DATA* 		pdat;		     // 現在のデータ参照用
    SVECTOR			vecRotTbl[64];	     // インデックス用

    // local param init
    nIndexNum = 0;
    nTypeNum  = 0;
    nTrapNum  = 0;
    nPointNum = 0;
    nAimNum   = 0;
    nRotNum   = 0;
    nIndexRotNum   = 0;

    // 初期化:(膨大だけど初期化しておく。改良あり？)
    for ( i = 0; i < FAT_C4_LEVEL; i++){
	for ( j = 0; j < FAT_C4_PATTERN; j++){
	    work->nPointTblUsed[i][j] = 0;
	}
    }
    for ( n = 0; n < FAT_C4_POSDATA; n++){
	pdat = &work->datPoint[i];
	pdat->nTrapName = -1;
	pdat->nType     = -1;
	pdat->nArea     = -1;
	pdat->nStatus   = 0;
	pdat->zoneAddr  = -1;
	pdat->vecPos    = DG_ZeroVector; 
	pdat->vecPut    = DG_ZeroVector; 
	pdat->vecRot    = DG_ZeroSVector; 
    }
    for ( n = 0; n < FAT_C4_LEVEL; n++){
	for ( j = 0; j < FAT_C4_PATTERN; j++){
	    for ( i = 0; i < FAT_C4_POSNUM; i++){
		work->nC4IndexData[n][j][i] = -1; // 設置場所データ
	    }
	}
    }
    // 設定データ
    if ( ( opt = GCL_GetOption( 'j' ) ) != NULL ){	
	if ( GCL_NextStr() != NULL )work->nBombPutMin   = GCL_GetNextInt();
	else			    work->nBombPutMin   = 2;
	if ( GCL_NextStr() != NULL )work->nBombPutMax   = GCL_GetNextInt();
	else			    work->nBombPutMax   = 4;
	if ( GCL_NextStr() != NULL )work->nBombBaseTime = COUNT_VMODE(GCL_GetNextInt());	// 最小設置時の爆発までのフレーム数
	else			    work->nBombBaseTime = COUNT_VMODE(2700);	
	if ( GCL_NextStr() != NULL )work->nBombAddTime  = COUNT_VMODE(GCL_GetNextInt());	// 一つ増える毎の加算フレーム数
	else			    work->nBombAddTime  = COUNT_VMODE(300);
    }else{
	work->nBombPutMin   = 2;
	work->nBombPutMax   = 4;
	work->nBombBaseTime = COUNT_VMODE(1800);	// 最小設置時の爆発までのフレーム数
	work->nBombAddTime  = COUNT_VMODE(900);		// 一つ増える毎の加算フレーム数
    }
//    printf(" %d %d %d %d\n", work->nBombPutMin, work->nBombPutMax, work->nBombBaseTime, work->nBombAddTime);

    /*-------------------- インデックス --------------------*/
    if ( ( opt = GCL_GetOption( 'i' ) ) != NULL ){	
	for ( n = 0; n < FAT_C4_LEVEL; n++){
	    for ( j = 0; j < FAT_C4_PATTERN; j++){
		for ( i = 0; i < FAT_C4_POSNUM; i++){
		    if ( GCL_NextStr() != NULL ){
			int nIndex;
			nIndex = GCL_GetNextInt();	// 種類
			work->nC4IndexData[n][j][i] = nIndex; // 設置場所インデックスデータ
			if ( work->nC4IndexData[n][j][i] >= FAT_C4_POSDATA ){
#ifdef DEBUG_MODE
MAO_PRINTF("fatman GCL data[-index] data overflow !!\n");
ASSERT(0);
#endif
			}
			nIndexNum++;
		    }else{
#ifdef DEBUG_MODE
MAO_PRINTF("fatman GCL data[-index] too few !!\n");
ASSERT(0);
#endif
			break;
		    }
		}
	    }
	}
    }else{
#ifdef DEBUG_MODE
MAO_PRINTF("fatman GCL data[-index] cannot find!!\n");
ASSERT(0);
#endif
    }

    /*-------------------- 種類 --------------------*/
    if ( ( opt = GCL_GetOption( 't' ) ) != NULL ){	
	for ( i = 0; i < FAT_C4_POSDATA; i++){
	    if ( GCL_NextStr() != NULL ){
		work->datPoint[i].nType = GCL_GetNextInt();	// 種類
		nTypeNum++;
	    }else{
		break;
	    }
	}
    }else{
#ifdef DEBUG_MODE
MAO_PRINTF("fatman GCL data[-type] cannot find!!\n");
ASSERT(0);
#endif
    }
    /*-------------------- トラップ名 --------------------*/
    if ( ( opt = GCL_GetOption( 'n' ) ) != NULL ){		
	for ( i = 0; i < FAT_C4_POSDATA; i++){
	    if ( GCL_NextStr() != NULL){
		work->datPoint[i].nTrapName = GCL_GetNextInt(); // トラップ名取得
		nTrapNum++;
	    }else{
		break;
	    }
	}
    }else{
#ifdef DEBUG_MODE
MAO_PRINTF("fatman GCL data[-nmtrap] cannot find!!\n");
ASSERT(0);
#endif
    }

    /*-------------------- 場所 --------------------*/
    if ( ( opt = GCL_GetOption( 'b' ) ) != NULL ){	
	for ( i = 0; i < FAT_C4_POSDATA; i++){
	    FVECTOR vecPos;
	    if ( ENE_GCL_GetFV( opt, &vecPos ) == 0 ){ // データがあるかどうか
		_sceVu0CopyVector( &work->datPoint[i].vecPos, &vecPos);
		nPointNum++;
	    }else{
		break;
	    }
	}
    }else{
#ifdef DEBUG_MODE
MAO_PRINTF("fatman GCL data[-bmpos] cannot find!!\n");
ASSERT(0);
#endif
    }
    work->nPutPosNum = nPointNum; // 設置ポイント数

    /*-------------------- 設置時にファットマンが立つ場所 --------------------*/
    if ( ( opt = GCL_GetOption( 'a' ) ) != NULL ){	
	for ( i = 0; i < FAT_C4_POSDATA; i++){
	    FVECTOR vecPut;
	    if ( ENE_GCL_GetFV( opt, &vecPut ) == 0 ){ // データがあるかどうか
		int g;
		HZX_Pos2Zone( &vecPut, &g, &work->datPoint[i].zoneAddr ); // ゾーン

		work->datPoint[i].nArea = FAT_GetArea( &vecPut ); 	  // エリア設定
		_sceVu0CopyVector( &work->datPoint[i].vecPut, &vecPut);	  // 位置
		nAimNum++;
	    }else{
		break;
	    }
	}
    }else{
#ifdef DEBUG_MODE
MAO_PRINTF("fatman GCL data[-aimpos] cannot find!!\n");
ASSERT(0);
#endif
    }
    /*-------------------- 回転情報 --------------------*/
    if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){	
	FVECTOR vecRot;
	for ( i = 0; i < 64; i++){
	    if ( ENE_GCL_GetFV( opt, &vecRot ) == 0 ){ // データがあるかどうか
		vecRotTbl[i].vx = (short)vecRot.vx;
		vecRotTbl[i].vy = (short)vecRot.vy;
		vecRotTbl[i].vz = (short)vecRot.vz;
		nRotNum++;
	    }else{
		break;
	    }
	}
    }else{ // 設定なし
#ifdef DEBUG_MODE
MAO_PRINTF("fatman GCL data[-rot] cannot find!!\n");
ASSERT(0);
#endif
    }

    if ( ( opt = GCL_GetOption( 'q' ) ) != NULL ){	
	int nIndex;
	for ( i = 0; i < FAT_C4_POSDATA; i++){
	    if ( GCL_NextStr() != NULL){
		nIndex = GCL_GetNextInt(); // 回転データ用インデックス
		work->datPoint[i].vecRot = vecRotTbl[nIndex];
		nIndexRotNum++;
	    }else{
		break;
	    }
	}
    }else{ // 設定なし
#ifdef DEBUG_MODE
MAO_PRINTF("fatman GCL data[-qrotindex] cannot find!!\n");
ASSERT(0);
#endif
    }

#ifdef DEBUG_MODE
printf("C4 GCL DATA--------- TOP\n");
printf("TYPE  : %d \n", nTypeNum);
printf("TRAP  : %d \n", nTrapNum);
printf("POINT : %d \n", nPointNum);
printf("AIM   : %d \n", nAimNum);
printf("ROT   : %d \n", nRotNum);
printf("INDEX : %d \n", nIndexRotNum);
printf("C4 GCL DATA--------- BOTTOM\n");
#endif

#ifdef DEBUG_MODE
MAO_PRINTF(">>>>>>>> Get FatmanC4-GCLData  End <<<<<<<<\n");
#endif
}
// Ｃ４設置場所取得:未設置の設置点からランダムで取得
static int FAT_GetC4PosRandom( Work* work )
{
    int i, nSeed, nIndex;
    FAT_PUTPOINT_DATA*	pputData;

    nSeed = MAO_Rand( FAT_GetCurrentC4PosNum(work) );
    for ( i = nSeed; i < FAT_GetCurrentC4PosNum(work) + nSeed; i++){
	nIndex = i % FAT_GetCurrentC4PosNum(work);
	pputData = FAT_GetCurrPosData( work, nIndex);
	// 未設置の設置点を返す
	if ( pputData->nStatus == 0 ){
	    return nIndex;
	}
    }
    return -1;
}

// Ｃ４設置場所取得:プレイヤーの対エリアを優先検索
static int FAT_GetC4PosAreaPrio( Work* 	work,	// ファットマンワーク      
				 int 	nArea)	// 基準となるエリア
{
    int i;
    int nSeed;
    int nIndex;
    int nAreaBit;
    FAT_PUTPOINT_DATA*	pputData;
    NPCWORK* npc;
    
    // ローカル初期化
    nIndex   = 0;
    nAreaBit = 0;
    npc      = &work->npc;

    // 検索
    nAreaBit = FAT_GetContAreaBit( nArea ); // 対となるエリア群をビットフラグ形式で取得
    nSeed = MAO_Rand( FAT_GetCurrentC4PosNum(work) );
    for ( i = nSeed; i < FAT_GetCurrentC4PosNum(work) + nSeed; i++){
	nIndex = i % FAT_GetCurrentC4PosNum(work);
	pputData = FAT_GetCurrPosData( work, nIndex);
	// 未設置で対となるエリアに存在している設置点を返す
	if ( pputData->nStatus == 0 && ((1 << pputData->nArea) & nAreaBit) ){
	    return nIndex;
	}
    }
    return -1; // 見つからず
}

// Ｃ４設置場所取得:距離優先で検索
static int FAT_GetC4PosDistPrio( Work* 	work,		// ファットマンワーク      
				 int	nJudgeDist)     // 判定距離
{
    int i;
    int nResIndex;
    NPCWORK* npc;
    FAT_PUTPOINT_DATA*	pputData;
    
    npc = &work->npc;
    nResIndex = -1;

    if ( nJudgeDist < 0){ // 一番離れている設置ポイントを返す
	int nTmpDist;
	int nCurrDist;

	nTmpDist  = 0;

	for ( i = 0; i < FAT_GetCurrentC4PosNum(work); i++){
	    pputData = FAT_GetCurrPosData( work, i);
	    // より遠いもの＆未設置を満たすＣ４を探す
	    if ( pputData->nStatus == 0 &&
		 (nCurrDist = HZX_ZoneDistance( GM_PlayerControl->hzx_id, HZX_Zone1(GM_PlayerControl->addr), 
						pputData->zoneAddr)) >= nTmpDist ){
		nResIndex = i;
		nTmpDist = nCurrDist;
	    }
	}
    }else{ // 指定距離以上の設置ポイントを返す
	int nSeed;
	int nIndex;

	nJudgeDist = nJudgeDist >> 8; // ゾーン距離に変換
	nSeed = MAO_Rand( FAT_GetCurrentC4PosNum(work) );
	for ( i = nSeed; i < FAT_GetCurrentC4PosNum(work) + nSeed; i++){
	    nIndex = i % FAT_GetCurrentC4PosNum(work);
	    pputData = FAT_GetCurrPosData( work, nIndex);
	    // 最低距離＆未設置を満たすＣ４を探す
	    if ( pputData->nStatus == 0 && HZX_ZoneDistance( GM_PlayerControl->hzx_id, 
		 HZX_Zone1(GM_PlayerControl->addr), pputData->zoneAddr) >= nJudgeDist ){
		return (nIndex);
	    }
	}
    }
    return nResIndex;
}
// Ｃ４設置場所取得:方向差優先検索
// nDiff >= 0 : 指定した方向差以内の設置ポイントを取得
// nDiff <  0 : 一番方向差の少ない設置ポイントを取得
static int FAT_GetC4PosDirPrio( Work* work, int nDiff )
{
    int i;
    int nResIndex;
    int nDir, nDirDiff, nDirDiffTmp;
    NPCWORK* npc;
    FAT_PUTPOINT_DATA*	pputData;

    npc = &work->npc;
    
    nResIndex = -1;
    nDir      = 0;
    if ( nDiff < 0){ // 一番方向差の少ない設置ポイントを取得
	nDirDiff = 4096;
	for ( i = 0; i < FAT_GetCurrentC4PosNum(work); i++){
	    pputData = FAT_GetCurrPosData( work, i);
	    if ( pputData->nStatus != 0){ continue; } // 未設置チェック
	    MAO_GetDiffVec3( NULL, NULL, &nDir, &npc->ctrl->mov, &pputData->vecPut ); // 方向取得
	    nDirDiffTmp = GV_DiffDirAbs( npc->ctrl->rot.vy, nDir); // 方向差取得
	    if ( nDirDiffTmp < nDirDiff ){ // 最も方向差の小さい設置ポイントにする
		nResIndex = i;
		nDirDiff = nDirDiffTmp;
	    }
	}
    }else{ // 指定方向差以内の設置ポイントを取得
	int nSeed;
	int nIndex;
	nDir  = 0;

	nSeed = MAO_Rand( FAT_GetCurrentC4PosNum(work) ); // ランダムで始点設定
	for ( i = nSeed; i < FAT_GetCurrentC4PosNum(work) + nSeed; i++){
	    nIndex = i % FAT_GetCurrentC4PosNum(work);
	    pputData = FAT_GetCurrPosData( work, nIndex);
	    if ( pputData->nStatus != 0 ){ continue; } // 未設置チェック
	    MAO_GetDiffVec3( NULL, NULL, &nDir, &npc->ctrl->mov, &pputData->vecPos ); // 方向取得
	    if ( GV_DiffDirAbs( npc->ctrl->rot.vy, nDir) < nDiff ){
		return (nIndex); // 指定方向差以内の設置ポイントを発見
	    }
	}
    }

    return (nResIndex);
}
// Ｃ４設置場所取得:指定種類を優先で取得
static int FAT_GetC4PosTypePrio( Work*  work,
				 int	nType)
{
    int i, nSeed, nIndex;
    FAT_PUTPOINT_DATA*	pputData;

    nSeed = MAO_Rand( FAT_GetCurrentC4PosNum(work) ); // ランダムで始点設定
    for ( i = nSeed; i < FAT_GetCurrentC4PosNum(work) + nSeed; i++){
	nIndex = i % FAT_GetCurrentC4PosNum(work);
	pputData = FAT_GetCurrPosData( work, nIndex);
	// 指定した種類で未設置の設置点を返す
	if ( pputData->nType == nType && pputData->nStatus == 0 ){
	    return nIndex;
	}
    }
    return -1;
}
// Ｃ４設置場所取得:現在選択しているインデックス以外を優先で取得
static int FAT_GetC4PosOtherPrio( Work* work )
{
    int i, nSeed, nIndex;
    FAT_PUTPOINT_DATA*	pputData;

    nSeed = work->nC4Index + 1; // 一つ後を始点として検索
    for ( i = nSeed; i < FAT_GetCurrentC4PosNum(work) + nSeed; i++){
	nIndex = i % FAT_GetCurrentC4PosNum(work);
	pputData = FAT_GetCurrPosData( work, nIndex);
	// 未設置の設置点を返す
	if ( pputData->nStatus == 0 ){
	    return nIndex;
	}
    }
    return -1;
}

#if 0
// 爆弾設置位置設定
static int FAT_SetNewPutIndex( Work* work, int nIndex )
{
    if ( !work->bPutIndexUsed && work->nC4Index != -1)
	return 0;

    work->nC4Index 	= nIndex;
    work->bPutIndexUsed = 0;

    return 1;
}
#endif
// 現在の爆弾設置点のデータへのポインタ取得
static FAT_PUTPOINT_DATA* FAT_GetCurrPutPosData( Work* work )
{
    return ( work->pCurPosData);
}

// 指定された難易度のパターン使用状況を更新する
static int FAT_UpdatePatternInfo( Work* work, int nDifficult)
{
    int j, n;
    FAT_PUTPOINT_DATA*	pput;		// 設置場所構造体へのポインタ

    for ( j = 0; j < FAT_C4_PATTERN; j++){
	work->nPointTblUsed[nDifficult][j] = 0; // クリア
	for ( n = 0; n < FAT_GetC4PosNum( work, nDifficult, j); n++){
	    pput = FAT_GetPosData( work, nDifficult, j, n);
	    if ( pput->nStatus > 0){ // 爆弾が存在するか？
		work->nPointTblUsed[nDifficult][j] = 1; // 使用フラグを立てる
		break;
	    }
	}
    }
    return 1;
}

// 難易度レベルで設置位置を設定
static int FAT_ChangeC4Difficult( Work* work, int nLevel)
{
    int nPreLevel;

    nPreLevel = work->nC4Level; // 失敗時用に退避
    work->nC4Level = nLevel; 

    if ( FAT_ChangeC4Pattern(work) ){ // C4のパターンを新規設定
	return 1; // 設定終了
    }else{
	// パターンチェンジができなかったので元に戻す
	work->nC4Level = nPreLevel; // 
    }

    return 0;
}

// パターンレベルで設置位置を設定
static int FAT_ChangeC4Pattern( Work* work)
{
    int i, j;
    int nSeed, nPattern;
    FAT_C4_DATA* 	pbombData;	// 爆弾情報構造体へのポインタ

    pbombData = &work->bombData[0];


    // パターン使用状況を更新
    FAT_UpdatePatternInfo( work, work->nC4Level);

    // パターン検索の初期位置を設定
    nSeed = MAO_Rand( FAT_C4_PATTERN );
    for ( j = nSeed; j < FAT_C4_PATTERN + nSeed; j++){
	nPattern = j % FAT_C4_PATTERN; 
	// 未使用で設置数を満たすパターンを返す
	if ( !work->nPointTblUsed[work->nC4Level][nPattern] ){
	    work->nC4Pattern = nPattern;
	    return 1;
	}
    }

    // C4を消してみる
    for ( i = 0; i < MAX_BOMB_NUM; i++){
	if ( ( pbombData + i )->nID != -1 &&
	     ( pbombData + i )->nStatus == -1){	// 凍結しているのワークがあるかチェック
	    // 凍結しているC4を除去
	    GM_DestroyKaitaiC4( ( pbombData + i )->nID );
	    // レーダー表示が残っていた
	    if ( ( pbombData + i )->pvRadar != NULL ){ 
		BOMB_Erase( ( pbombData + i )->pvRadar, BOMB_RADAR_FEED_OUT);
		( pbombData + i )->pvRadar = NULL;
	    }
	}
    }
    // 再度検索
    nSeed = MAO_Rand( FAT_C4_PATTERN );
    for ( j = nSeed; j < FAT_C4_PATTERN + nSeed; j++){
	nPattern = j % FAT_C4_PATTERN; 
	// 未使用で設置数を満たすパターンを返す
	if ( !work->nPointTblUsed[work->nC4Level][nPattern] ){
	    work->nC4Pattern = nPattern;
	    return 1;
	}
    }


    return 0;
}
// インデックスレベルで設置位置を選定
static int FAT_ChangeC4Index( Work* work, 	// ファットマンワーク
			      int nSearch, 	// 検索方法
			      int nParam)	// 検索パラメータ:検索方法で内容は異なる
{
    int nIndex;
    nIndex = -1;

    // 検索方法で分岐
    switch ( nSearch ){
    case FAT_C4_SEARCH_AREA:  // 指定エリアの対エリアを優先検索
	nIndex = FAT_GetC4PosAreaPrio( work, nParam );
	if ( nIndex < 0 ) nIndex = FAT_GetC4PosRandom( work );
	if ( nIndex < 0 ) return -1;
	break;
    case FAT_C4_SEARCH_DIST:  // 指定距離以上を優先検索
	nIndex = FAT_GetC4PosDistPrio( work, nParam );
	if ( nIndex < 0 ) nIndex = FAT_GetC4PosRandom( work );
	if ( nIndex < 0 ) return -1;
	break;
    case FAT_C4_SEARCH_DIR:   // 指定方向差を優先検索
	nIndex = FAT_GetC4PosDirPrio( work, nParam );
	if ( nIndex < 0 ) nIndex = FAT_GetC4PosRandom( work );
	if ( nIndex < 0 ) return -1;
	break;
    case FAT_C4_SEARCH_TYPE:  // 指定種類を優先検索
	nIndex = FAT_GetC4PosTypePrio( work, nParam );
	if ( nIndex < 0 ) nIndex = FAT_GetC4PosRandom( work );
	if ( nIndex < 0 ) return -1;
	break;
    case FAT_C4_SEARCH_OTHER: // 今の設置ポイント以外を優先
	nIndex = FAT_GetC4PosOtherPrio( work );
	if ( nIndex < 0 ) nIndex = FAT_GetC4PosRandom( work );
	if ( nIndex < 0 ) return -1;
	break;
    }

    return nIndex; // 成功
}


// 設置ポイントサーチ
#define FAT_C4_INTV_SNIT    (COUNT_VMODE(600)) // なかなか置けなくていらつく時間
static int FAT_SetNewC4Point( Work* work)
{
    int 		nRand;
    int 		nIndex;
    NPCWORK* 		npc;
    FAT_PUTPOINT_DATA*  pput;

    npc = &work->npc;
    
    nIndex = -1;
    if ( work->nPutNum == 0){ // 	一つ目
	if ( work->nPutIntvCntr >= FAT_C4_INTV_SNIT ){ // いらついている
	    nIndex = FAT_ChangeC4Index( work, FAT_C4_SEARCH_TYPE, FAT_TYPE_RUN); // 走り置き優先
	}else{
	    // 最初なので検索方法はランダム選択
	    nRand = MAO_Rand( 10 );
	    if ( nRand < 2 ){
		nIndex = FAT_ChangeC4Index( work, FAT_C4_SEARCH_AREA, work->nPlaArea); // プレイヤー対エリア優先
	    }else if ( nRand < 4 ){
		nIndex = FAT_ChangeC4Index( work, FAT_C4_SEARCH_DIST, 3000); 	       // 一定距離離れたポイント優先
	    }else if ( nRand < 7 ){
		nIndex = FAT_ChangeC4Index( work, FAT_C4_SEARCH_TYPE, FAT_TYPE_RUN);   // 走り置き優先
	    }else {
		nIndex = FAT_ChangeC4Index( work, FAT_C4_SEARCH_TYPE, FAT_TYPE_STAND); // 立ち置き優先
	    }
	}
    }else{
	if ( work->nPutIntvCntr >= FAT_C4_INTV_SNIT ){ // いらついている
	    nIndex = FAT_ChangeC4Index( work, FAT_C4_SEARCH_TYPE, FAT_TYPE_RUN); // 走り置き優先
	}else{
	    // 最初なので検索方法はランダム選択
	    nRand = MAO_Rand( 10 );
	    if ( nRand < 2 ){
		nIndex = FAT_ChangeC4Index( work, FAT_C4_SEARCH_AREA, work->nPlaArea); // プレイヤー対エリア優先
	    }else{
		nIndex = FAT_ChangeC4Index( work, FAT_C4_SEARCH_DIST, 3000); 	       // 一定距離離れたポイント優先
	    }
	}
    }

    // まだ有効にならない
    if (nIndex < 0){	
	nIndex = FAT_ChangeC4Index( work, FAT_C4_SEARCH_OTHER, 0); // 今のインデックス以外を優先
    }

#ifdef DEBUG_MODE
MAO_PRINTF("BOMB INDEX = %d ", nIndex);
#endif

    // インデックスが有効な時は反映させる
    if ( nIndex >= 0 ){	
	pput = FAT_GetCurrPosData( work, nIndex); // 設置場所取得

	if ( pput == NULL || pput->nStatus != 0){ // 不正チェック
#ifdef DEBUG_MODE
MAO_PRINTF("@@@@@@@@@@@ Bomb Already Exist !!!! @@@@@@@@@@@\n");
GV_ERROR( GV_ERROR_WORLD_BOTTOM ) ;
#endif
	    return 0;
	}

	work->nC4Index 	  = nIndex;
	work->pCurPosData = pput;

	if ( pput->nType & FAT_TYPE_RUN ){ // 走り置きタイプ
	    FAT_SetNewC4Position( work, nIndex);
	    FAT_SetThink3( work, TH3_POINT_MOVE);	// 思考準備へ
	    work->nRunningPut = 1;			// 走り置きフラグON
	    return 1;
	}else{
	    FAT_SetNewC4Position( work, nIndex);
	    FAT_SetThink3( work, TH3_POINT_MOVE);		// 爆弾設置移動	
	    work->nRunningPut = 0;
	    return 1;
	}
    }else{
#ifdef DEBUG_MODE
MAO_PRINTF("Cannot find Bomb Put area !!!!\n");
#endif
    }

    return 0;
}


// C4設置位置設定
static int FAT_SetNewC4Position( Work* work, int nIndex )
{
    work->pCurPosData =  FAT_GetCurrPosData( work, nIndex); // 設置場所取得

    // 新しい目標を設定
    FAT_SetNewAim( work, HZX_Zone1( work->pCurPosData->zoneAddr), &work->pCurPosData->vecPut);

    return 0;
}

// C4設置ゾーン取得
static int FAT_GetCurrentC4Zone( Work* work)
{
    return ( HZX_Zone1( work->pCurPosData->zoneAddr) );
}
// C4設置立ち場所取得
static void FAT_GetCurrentC4Put( Work* work, FVECTOR* pvec)
{
    _sceVu0CopyVector( pvec, &work->pCurPosData->vecPut);
}
// C4設置場所取得
static void FAT_GetCurrentC4Pos( Work* work, FVECTOR* pvec)
{
    _sceVu0CopyVector( pvec, &work->pCurPosData->vecPos);
}

// 回転置きの方向差分
static int FAT_GetTurnPutDir( Work* work)
{
    int 	nDirIn, nDirMine;
    int		nNextZone;	
    FVECTOR	vecPut, vecPos;
    NPCWORK*	npc;
    npc = &work->npc;

    // 突入方向取得
    nNextZone = HZX_NextZone( npc->ctrl->hzx_id, FAT_GetCurrentC4Zone( work), HZX_Zone1(npc->ctrl->addr) );
    nDirIn = MAO_GetDirZoneToZone( nNextZone, FAT_GetCurrentC4Zone( work) );

    // 設置立ち位置->設置位置の方向を求める
    FAT_GetCurrentC4Put( work, &vecPut);
    FAT_GetCurrentC4Pos( work, &vecPos);
    MAO_GetDiffVec3( NULL, NULL, &nDirMine, &vecPut, &vecPos);

    return ( GV_DiffDirS( nDirIn, nDirMine) ); // 方向の差分を返す
}

// アクティブなC4が画面内にあるかチェック
static int FAT_VisibleC4Check( Work* work )
{
    int i;
    NPCWORK*	npc;
    FAT_C4_DATA* 	pbombData;
    static FVECTOR C4_SIZE_MIN = { -250.f, -300.f, -150.f, 1.f };
    static FVECTOR C4_SIZE_MAX = {  250.f,  300.f,  150.f, 1.f };

    npc = &work->npc;
    pbombData = work->bombData;

    for ( i = 0; i < MAX_BOMB_NUM; i++){
	if ( ( pbombData + i )->nID != -1 && ( pbombData + i )->nStatus > 0 ){	// アクティブなC4を探す
	    if ( DG_BoundCheck( &( pbombData + i )->matPos, &C4_SIZE_MAX, &C4_SIZE_MIN ) != -1 ){ // 画面チェック
		FVECTOR vecFrom, vecTo, vecAdd;
		_sceVu0CopyVector( &vecFrom, (FVECTOR*)&DG_Chanls[0].eye.m[3][0]);
		_sceVu0CopyVector( &vecTo, (FVECTOR* )&( pbombData + i )->matPos.m[3][0]);
		_sceVu0ScaleVector( &vecAdd, (FVECTOR* )&( pbombData + i )->matPos.m[2][0], 50.f);
		_sceVu0AddVector( &vecTo, &vecTo, &vecAdd);
		if ( !ENE_EyeOnlineCheck( GM_PlayerControl->hzx_id, &vecFrom, &vecTo) ){ // 可視チェック
		    return 1;
		}
	    }
	}	
    }

    return 0;
}

// 解体中のC4のデータを取得
static int FAT_SearchKaitaiC4( Work* work, FAT_C4_DATA* pbombRes )
{
    int i, nDist;
    NPCWORK*	npc;
    FVECTOR	vecDiff;
    FAT_C4_DATA* 	pbombData;

    static FVECTOR C4_SIZE_MIN = { -250.f, -300.f, -150.f, 1.f };
    static FVECTOR C4_SIZE_MAX = {  250.f,  300.f,  150.f, 1.f };

    // スプレーを発射している状態かどうか
    if ( PL_CurrentWeapon() != WP_ColdSpray || GM_WeaponFire != WP_ColdSpray ) return 0;

    npc = &work->npc;
    pbombData = work->bombData;

    for ( i = 0; i < MAX_BOMB_NUM; i++){
	if ( ( pbombData + i )->nID != -1 && ( pbombData + i )->nStatus > 0 ){	// アクティブなC4を探す
	    if ( DG_BoundCheck( &( pbombData + i )->matPos, &C4_SIZE_MAX, &C4_SIZE_MIN ) != -1 ){ // 画面チェック
		FVECTOR vecFrom, vecTo, vecAdd;
		_sceVu0CopyVector( &vecFrom, (FVECTOR*)&GM_PlayerControl->mov);
		_sceVu0CopyVector( &vecTo, (FVECTOR* )&( pbombData + i )->matPos.m[3][0]);
		_sceVu0ScaleVector( &vecAdd, (FVECTOR* )&( pbombData + i )->matPos.m[2][0], 50.f);
		_sceVu0AddVector( &vecTo, &vecTo, &vecAdd);

		// プレイヤーと爆弾の位置関係を取得
		MAO_GetDiffVec3( &vecDiff, &nDist, NULL, (FVECTOR* )&GM_PlayerControl->mov, (FVECTOR* )&( pbombData + i )->matPos.m[3][0]);
		if ( _sceVu0InnerProduct( (FVECTOR* )&DG_Chanls[0].eye.m[2][0], &vecDiff) > 0.9f && nDist < 2400.f &&
		     !ENE_EyeOnlineCheck( GM_PlayerControl->hzx_id, &vecFrom, &vecTo) ){ // 方向,距離,可視チェック
		    (*pbombRes) = *(work->bombData + i);
		    return 1;
		}
	    }
	}	
    }

    return 0;
}

// カウントダウン中の爆弾からプレイヤーから最も遠い物を探す
static FAT_C4_DATA* FAT_SearchKaitaiC4FarPlayer( Work* work )
{
    int i, nDist, nDistTmp;
    NPCWORK*	npc;
    FAT_C4_DATA* 	pbombData;
    FAT_C4_DATA*	pdat;	

    npc = &work->npc;
    pbombData = work->bombData;

    nDist = 0;
    pdat = NULL;

    for ( i = 0; i < MAX_BOMB_NUM; i++){
	if ( ( pbombData + i )->nID != -1 && ( pbombData + i )->nStatus > 0 ){	// アクティブなC4を探す
	    nDistTmp = (int)_MAO_FVec2Len2( &GM_PlayerPosition, (FVECTOR*)&( pbombData + i )->matPos.m[3][0]);
	    if ( nDistTmp >= nDist){
		pdat = ( pbombData + i );
		nDist = nDistTmp;
	    }
	}	
    }

    return pdat;
}
// カウントダウン中の爆弾からプレイヤーから最も近い物を探す
static int FAT_SearchKaitaiC4DistNearest( Work* work )
{
    int i, nDist, nDistTmp;
    NPCWORK*	npc;
    FAT_C4_DATA* 	pbombData;

    npc = &work->npc;
    pbombData = work->bombData;

    nDist = 60000;

    for ( i = 0; i < MAX_BOMB_NUM; i++){
	if ( ( pbombData + i )->nID != -1 && ( pbombData + i )->nStatus > 0 ){	// アクティブなC4を探す
	    nDistTmp = (int)_MAO_FVec2Len2( &GM_PlayerPosition, (FVECTOR*)&( pbombData + i )->matPos.m[3][0]);
	    if ( nDistTmp < nDist){
		nDist = nDistTmp;
	    }
	}	
    }

    return (nDist);
}
