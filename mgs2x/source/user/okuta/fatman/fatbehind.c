/*
    fatbehind.c
    ファットマン戦銃撃フェーズで使用する対ビハインドルーチン
    2001/04/11 Masafumi Okuta
    $Id: fatbehind.c,v 1.1.1.3 2002/11/19 11:47:56 Yoshizawa1 Exp $
*/


// ファットマン戦銃撃フェーズで使用する対ビハインド用位置取りデータ
typedef struct {
    FVECTOR  vecPos;	// プレイヤーポイント
    FVECTOR  vecBack;	// 背面ポイント
    FVECTOR  vecCorner;	// 対角ポイント
    FVECTOR  vecShoot;	// 飛び出しポイント
    FVECTOR  vecHard;	// ハードモード用回り込み目標ポイント
}FAT_BEHIND_VECDATA;

// ファットマン戦銃撃フェーズで使用する対ビハインド用バウンドボックスデータ
typedef struct {
    FVECTOR  vecLTop;		// 左上
    FVECTOR  vecLBottom;	// 左下
    FVECTOR  vecRTop;		// 右上
    FVECTOR  vecRBottom;	// 右下
    FVECTOR  vecCenter;		// 中心
}FAT_BOUND_VECDATA;

enum{	// ファットマン４象限チェック用
FAT_BEHIND_QUAD_NON,	// なし
FAT_BEHIND_QUAD_LT,	// 左上
FAT_BEHIND_QUAD_LB,	// 左下
FAT_BEHIND_QUAD_RT,	// 右上
FAT_BEHIND_QUAD_RB,	// 右下
};

enum{	// ビハインド方向
FAT_BEHIND_DIR_U	= 0x00000001,	// 上
FAT_BEHIND_DIR_D	= 0x00000002,	// 下
FAT_BEHIND_DIR_UD	= (FAT_BEHIND_DIR_U | FAT_BEHIND_DIR_D), // 上下
FAT_BEHIND_DIR_L	= 0x00000004,	// 左
FAT_BEHIND_DIR_R	= 0x00000008,	// 右
FAT_BEHIND_DIR_LR	= (FAT_BEHIND_DIR_L | FAT_BEHIND_DIR_R), // 左右
FAT_BEHIND_DIR_CHECK	= 0x0000000f	// 方向チェック用	
};

enum{ // ビハインド対応データ取得用
FAT_BEHIND_GET_BACK,	// 背面ポイント取得
FAT_BEHIND_GET_CORNER,	// 対角ポイント取得
FAT_BEHIND_GET_SHOOT,	// 攻撃ポイント取得
FAT_BEHIND_GET_HARD,	// ハード用ポイント取得
};

enum{// ビハインド対応データの有効フラグ
FAT_BH_VALID_B  	= 0x00000010,	// 背面位置移動可能
FAT_BH_VALID_C  	= 0x00000020,	// 対角位置移動可能
FAT_BH_VALID_BC		= 0x00000030,	//
FAT_BH_VALID_S  	= 0x00000040,	// 攻撃位置移動可能
FAT_BH_VALID_BS		= 0x00000050,	//
FAT_BH_VALID_CS		= 0x00000060,	//
FAT_BH_VALID_BCS	= 0x00000070,	//
FAT_BH_VALID_H  	= 0x00000080,	// ハードモード用位置移動可能
FAT_BH_VALID_BH		= 0x00000090,	//
FAT_BH_VALID_CH		= 0x000000A0,	//
FAT_BH_VALID_BCH	= 0x000000B0,	//
FAT_BH_VALID_SH		= 0x000000C0,	//
FAT_BH_VALID_BSH	= 0x000000D0,	//
FAT_BH_VALID_CSH	= 0x000000E0,	//
FAT_BH_VALID_BCSH	= 0x000000F0,	//
FAT_BH_VALID_CHECK	= 0x000000F0,	// 取得用
};

#define DBG_BOUNDBOX_SIZE (250.f)

// ４象限分割でバウンドチェック
static int FvecBoundCheckQuad(FVECTOR *pvecLTop,  	// 左上
			      FVECTOR *pvecRBottom,  	// 右下
			      FVECTOR *pvecCenter,	// 中心
			      FVECTOR *pvecTrg)  	// 判定物
{
    if ((pvecLTop->vy <= pvecTrg->vy ) && (pvecTrg->vy <= pvecRBottom->vy )){ // 高さをチェック
	if ( (pvecLTop->vx <= pvecTrg->vx) && (pvecTrg->vx < pvecCenter->vx) ){ // 左半分
	    if ( (pvecLTop->vz <= pvecTrg->vz) && (pvecTrg->vz < pvecCenter->vz) ){ // 上
		return (FAT_BEHIND_QUAD_LT);
	    }else if ( (pvecCenter->vz <= pvecTrg->vz) && (pvecTrg->vz < pvecRBottom->vz) ){ // 下	
		return (FAT_BEHIND_QUAD_LB);
	    }
	}else if ( (pvecCenter->vx <= pvecTrg->vx) && (pvecTrg->vx <= pvecRBottom->vx) ){ // 右半分
	    if ( (pvecLTop->vz <= pvecTrg->vz) && (pvecTrg->vz < pvecCenter->vz) ){ // 上
		return (FAT_BEHIND_QUAD_RT);
	    }else if ( (pvecCenter->vz <= pvecTrg->vz) && (pvecTrg->vz < pvecRBottom->vz) ){ // 下
		return (FAT_BEHIND_QUAD_RB);
	    }
	}
    }
    return (FAT_BEHIND_QUAD_NON);
}
// ビハインドデータを取得
static void FAT_GetBehindData(int		  nBoundQuad,	// バウンド象限
			      int		  nBehindDir,	// ビハインド方向
			      FAT_BOUND_VECDATA*  pvdatBound,	// バウンドデータ
			      FAT_BEHIND_VECDATA* pvdatBehind ) // ビハインドデータ:結果がここに入る
{
    if ( (nBehindDir & FAT_BEHIND_DIR_UD)){
	switch( nBoundQuad ){
	case FAT_BEHIND_QUAD_LT:	// 左上
	    pvdatBehind->vecPos 	= pvdatBound->vecLTop;
	    pvdatBehind->vecBack 	= pvdatBound->vecLBottom;
	    pvdatBehind->vecCorner	= pvdatBound->vecRBottom;
	    pvdatBehind->vecHard 	= pvdatBound->vecRTop;
	    break;
	case FAT_BEHIND_QUAD_LB:	// 左下
	    pvdatBehind->vecPos 	= pvdatBound->vecLBottom;
	    pvdatBehind->vecBack 	= pvdatBound->vecLTop;
	    pvdatBehind->vecCorner	= pvdatBound->vecRTop;
	    pvdatBehind->vecHard 	= pvdatBound->vecRBottom;
	    break;
	case FAT_BEHIND_QUAD_RT:	// 右上
	    pvdatBehind->vecPos 	= pvdatBound->vecRTop;
	    pvdatBehind->vecBack 	= pvdatBound->vecRBottom;
	    pvdatBehind->vecCorner	= pvdatBound->vecLBottom;
	    pvdatBehind->vecHard 	= pvdatBound->vecLTop;
	    break;
	case FAT_BEHIND_QUAD_RB:	// 右下
	    pvdatBehind->vecPos 	= pvdatBound->vecRBottom;
	    pvdatBehind->vecBack 	= pvdatBound->vecRTop;
	    pvdatBehind->vecCorner	= pvdatBound->vecLTop;
	    pvdatBehind->vecHard 	= pvdatBound->vecLBottom;
	    break;
	}
	pvdatBehind->vecShoot 	= pvdatBound->vecCenter;
    }else{
	switch( nBoundQuad ){
	case FAT_BEHIND_QUAD_LT:	// 左上
	    pvdatBehind->vecPos 	= pvdatBound->vecLTop;
	    pvdatBehind->vecBack 	= pvdatBound->vecRTop;
	    pvdatBehind->vecCorner	= pvdatBound->vecRBottom;
	    pvdatBehind->vecHard 	= pvdatBound->vecLBottom;
	    break;
	case FAT_BEHIND_QUAD_LB:	// 左下
	    pvdatBehind->vecPos 	= pvdatBound->vecLBottom;
	    pvdatBehind->vecBack 	= pvdatBound->vecRBottom;
	    pvdatBehind->vecCorner	= pvdatBound->vecRTop;
	    pvdatBehind->vecHard 	= pvdatBound->vecLTop;
	    break;
	case FAT_BEHIND_QUAD_RT:	// 右上
	    pvdatBehind->vecPos 	= pvdatBound->vecRTop;
	    pvdatBehind->vecBack 	= pvdatBound->vecLTop;
	    pvdatBehind->vecCorner	= pvdatBound->vecLBottom;
	    pvdatBehind->vecHard 	= pvdatBound->vecRBottom;
	    break;
	case FAT_BEHIND_QUAD_RB:	// 右下
	    pvdatBehind->vecPos 	= pvdatBound->vecRBottom;
	    pvdatBehind->vecBack 	= pvdatBound->vecLBottom;
	    pvdatBehind->vecCorner	= pvdatBound->vecLTop;
	    pvdatBehind->vecHard 	= pvdatBound->vecRTop;
	    break;
	}
	pvdatBehind->vecShoot 	= pvdatBound->vecCenter;
    }
}
#if 0
// デバック
static void FAT_DbgDrawBehindData( FAT_BEHIND_VECDATA* pvdatBehind )
{
    FVECTOR vec1;
    FVECTOR vec2;

    // 背面
    vec1.vx = pvdatBehind->vecBack.vx - DBG_BOUNDBOX_SIZE;
    vec1.vy = 12426.f - DBG_BOUNDBOX_SIZE;
    vec1.vz = pvdatBehind->vecBack.vz - DBG_BOUNDBOX_SIZE;
    vec2.vx = pvdatBehind->vecBack.vx + DBG_BOUNDBOX_SIZE;
    vec2.vy = 12426.f + DBG_BOUNDBOX_SIZE;
    vec2.vz = pvdatBehind->vecBack.vz + DBG_BOUNDBOX_SIZE;
    NewBoundingBoxView( &vec1, &vec2, 0, 255, 255);

    // 対角
    vec1.vx = pvdatBehind->vecCorner.vx - DBG_BOUNDBOX_SIZE;
    vec1.vy = 12426.f - DBG_BOUNDBOX_SIZE;
    vec1.vz = pvdatBehind->vecCorner.vz - DBG_BOUNDBOX_SIZE;
    vec2.vx = pvdatBehind->vecCorner.vx + DBG_BOUNDBOX_SIZE;
    vec2.vy = 12426.f + DBG_BOUNDBOX_SIZE;
    vec2.vz = pvdatBehind->vecCorner.vz + DBG_BOUNDBOX_SIZE;
    NewBoundingBoxView( &vec1, &vec2, 255, 0, 0);

    // 飛び出し
    vec1.vx = pvdatBehind->vecShoot.vx - DBG_BOUNDBOX_SIZE;
    vec1.vy = 12426.f - DBG_BOUNDBOX_SIZE;
    vec1.vz = pvdatBehind->vecShoot.vz - DBG_BOUNDBOX_SIZE;
    vec2.vx = pvdatBehind->vecShoot.vx + DBG_BOUNDBOX_SIZE;
    vec2.vy = 12426.f + DBG_BOUNDBOX_SIZE;
    vec2.vz = pvdatBehind->vecShoot.vz + DBG_BOUNDBOX_SIZE;
    NewBoundingBoxView( &vec1, &vec2, 0, 255, 0);

    // 回り込み
    vec1.vx = pvdatBehind->vecHard.vx - DBG_BOUNDBOX_SIZE;
    vec1.vy = 12426.f - DBG_BOUNDBOX_SIZE;
    vec1.vz = pvdatBehind->vecHard.vz - DBG_BOUNDBOX_SIZE;
    vec2.vx = pvdatBehind->vecHard.vx + DBG_BOUNDBOX_SIZE;
    vec2.vy = 12426.f + DBG_BOUNDBOX_SIZE;
    vec2.vz = pvdatBehind->vecHard.vz + DBG_BOUNDBOX_SIZE;
    NewBoundingBoxView( &vec1, &vec2, 255, 0, 255);
}
#endif
// プレイヤーが指定ボックスに入っているかチェック
// -1 		  : 失敗
// インデックス値 : 成功
static int FAT_CheckPlayerBehindBox(FAT_BEHIND_UNIT*	        unit,		 // ビハインドワーク
				    FAT_BEHIND_VECDATA* pvdatBehind )   // ビハインドデータが入る
{
    int i;
    int nDir, nDirFlag;
    int nCheckDir;
    int nBoundRes;

    nDir = 0;
    nDirFlag = 0;
    nCheckDir = 0;

    if ( GM_PlayerStatus & PLAYER_CAUTION ){ // ビハインド中
	nDir = GM_PlayerControl->rot.vy % 4096;
	if ( nDir >= 4096 - 256 || nDir <= 256){
	    nDirFlag |= FAT_BEHIND_DIR_D;
	    nCheckDir |= FAT_BEHIND_DIR_UD;
	}else if ( nDir >= 1024 - 256 && nDir <= 1024 + 256 ){
	    nDirFlag |= FAT_BEHIND_DIR_R;
	    nCheckDir |= FAT_BEHIND_DIR_LR;
	}else if ( nDir >= 2048 - 256 && nDir <= 2048 + 256 ){
	    nDirFlag |= FAT_BEHIND_DIR_U;
	    nCheckDir |= FAT_BEHIND_DIR_UD;
	}else if ( nDir >= 3072 - 256 && nDir <= 3072 + 256 ){
	    nDirFlag |= FAT_BEHIND_DIR_L;
	    nCheckDir |= FAT_BEHIND_DIR_LR;
	}
    }else{
	return (-1);	// 失敗
    }

    for ( i = 0; i < unit->nDataNum; i++ ){
	if ( !(unit->nType[i] & nCheckDir) ) {
	    continue;
	}
	nBoundRes = FvecBoundCheckQuad( &unit->vdatArea[i].vecLTop, &unit->vdatArea[i].vecRBottom, 
					&unit->vdatArea[i].vecCenter, &GM_PlayerPosition);
	if ( nBoundRes != 0){ // 内包された
//printf("%d : 0x%08x\n", nBoundRes, nDirFlag);
	    // ビハインド用データ取得
	    FAT_GetBehindData( nBoundRes, nDirFlag, &unit->vdatPos[i], pvdatBehind);
	    
//	    FAT_DbgDrawBehindData(pvdatBehind);
	    return (i);
	}
    }
    return (-1);
}

// GCLからデータ取得
static void FAT_GetGclBehindData( FAT_BEHIND_UNIT* unit )
{
    char* opt;
    int	  i;
    int   nTypeNum;
    int   nAreaNum;
    int   nPosNum;

    nTypeNum = 0;
    nAreaNum = 0;
    nPosNum = 0;

    /*-------------------- 種類 --------------------*/
    if ( ( opt = GCL_GetOption( 'c' ) ) != NULL ){	
	for ( i = 0; i < FAT_BEHIND_DATA_NUM; i++){
	    if ( GCL_NextStr() != NULL ){
		unit->nType[i] = GCL_GetNextInt();
		nTypeNum++;
	    }else{
		break;
	    }
	}
	for ( i = nTypeNum; i < FAT_BEHIND_DATA_NUM; i++){
	    unit->nType[i] = 0;
	}
    }
    /*-------------------- エリア判定用データ --------------------*/
    if ( ( opt = GCL_GetOption( 'h' ) ) != NULL ){	
	for ( i = 0; i < FAT_BEHIND_DATA_NUM; i++){
	    if ( ENE_GCL_GetFV( opt, &unit->vdatArea[i].vecLTop ) == 0 ){ 	 // 左上の座標
		if ( ENE_GCL_GetFV( opt, &unit->vdatArea[i].vecRBottom ) == 0 ){ // 右下の座標
		    nAreaNum++;      
		}else{
printf("Irrigal Data Num!! fatbehind : line %d\n", __LINE__);
ASSERT(0);
                    break;
		}
	    }else{
		break;
	    }
	}
	for ( i = 0; i < nAreaNum; i++){
	    // 左下の座標生成
	    unit->vdatArea[i].vecLBottom.vx = unit->vdatArea[i].vecLTop.vx;
	    unit->vdatArea[i].vecLBottom.vy = unit->vdatArea[i].vecRBottom.vy;
	    unit->vdatArea[i].vecLBottom.vz = unit->vdatArea[i].vecRBottom.vz;
	    // 右下の座標生成
	    unit->vdatArea[i].vecRTop.vx = unit->vdatArea[i].vecRBottom.vx;
	    unit->vdatArea[i].vecRTop.vy = unit->vdatArea[i].vecLTop.vy;
	    unit->vdatArea[i].vecRTop.vz = unit->vdatArea[i].vecLTop.vz;
	    // 中心の座標生成
	    unit->vdatArea[i].vecCenter.vx = (unit->vdatArea[i].vecLTop.vx + unit->vdatArea[i].vecRBottom.vx) * 0.5f;
	    unit->vdatArea[i].vecCenter.vy = (unit->vdatArea[i].vecLTop.vy + unit->vdatArea[i].vecRBottom.vy) * 0.5f;
	    unit->vdatArea[i].vecCenter.vz = (unit->vdatArea[i].vecLTop.vz + unit->vdatArea[i].vecRBottom.vz) * 0.5f;
	}
    }
    
    /*-------------------- 位置データ --------------------*/
    if ( ( opt = GCL_GetOption( 'm' ) ) != NULL ){	
	for ( i = 0; i < FAT_BEHIND_DATA_NUM; i++){
	    if ( ENE_GCL_GetFV( opt, &unit->vdatPos[i].vecLTop ) == 0 ){ 	 // 左上の座標
		if ( ENE_GCL_GetFV( opt, &unit->vdatPos[i].vecRBottom ) == 0 ){ // 右下の座標
		    nPosNum++;      
		}else{
printf("Irrigal Data Num!! fatbehind : line %d\n", __LINE__);
ASSERT(0);
                    break;
		}
	    }else{
		break;
	    }
	}
	for ( i = 0; i < nPosNum; i++){
	    // 左下の座標生成
	    unit->vdatPos[i].vecLBottom.vx = unit->vdatPos[i].vecLTop.vx;
	    unit->vdatPos[i].vecLBottom.vy = unit->vdatPos[i].vecRBottom.vy;
	    unit->vdatPos[i].vecLBottom.vz = unit->vdatPos[i].vecRBottom.vz;
	    // 右下の座標生成
	    unit->vdatPos[i].vecRTop.vx = unit->vdatPos[i].vecRBottom.vx;
	    unit->vdatPos[i].vecRTop.vy = unit->vdatPos[i].vecLTop.vy;
	    unit->vdatPos[i].vecRTop.vz = unit->vdatPos[i].vecLTop.vz;
	    // 中心の座標生成
	    unit->vdatPos[i].vecCenter.vx = (unit->vdatPos[i].vecLTop.vx + unit->vdatPos[i].vecRBottom.vx) * 0.5f;
	    unit->vdatPos[i].vecCenter.vy = (unit->vdatPos[i].vecLTop.vy + unit->vdatPos[i].vecRBottom.vy) * 0.5f;
	    unit->vdatPos[i].vecCenter.vz = (unit->vdatPos[i].vecLTop.vz + unit->vdatPos[i].vecRBottom.vz) * 0.5f;
	}
    }
printf("BEHIND GCL DATA--------- TOP\n");
printf("TYPE  : %d \n", nTypeNum);
printf("POS   : %d \n", nPosNum);
printf("AREA  : %d \n", nAreaNum);
printf("BEHIND GCL DATA--------- BOTTOM\n");

    ASSERT( nTypeNum == nPosNum );	// データ数が違う
    ASSERT( nTypeNum == nAreaNum );	// データ数が違う
    unit->nDataNum = nTypeNum; 		// データ数設定 
}
// プレイヤービハインドに対応した指定形式の座標を取得
static int FAT_GetBehindContPoint( Work* work, FVECTOR* pvecGet, int nGetFlag )
{
    int nIndex;
    int nValidFlag;
    FAT_BEHIND_UNIT* unit;
    FAT_BEHIND_VECDATA vdatBehind;	

    unit = &work->unitBehind;

    if ( (nIndex = FAT_CheckPlayerBehindBox( unit, &vdatBehind)) == -1){
	return 0; // 失敗
    }

    // 有効フラグ取得
    nValidFlag = ( unit->nType[nIndex] & FAT_BH_VALID_CHECK );

    switch (nGetFlag){
    case FAT_BEHIND_GET_BACK:
	if ( !(nValidFlag & FAT_BH_VALID_B) ) return 0;
	(*pvecGet) = vdatBehind.vecBack;
	work->vecCautionPos = vdatBehind.vecPos;
//printf("back = %f %f %f\n", (*pvecGet).vx, (*pvecGet).vy, (*pvecGet).vz);
	return 1;
    case FAT_BEHIND_GET_CORNER:
	if ( !(nValidFlag & FAT_BH_VALID_C) ) return 0;
	(*pvecGet) = vdatBehind.vecCorner;
	work->vecCautionPos = vdatBehind.vecPos;
//printf("corn = %f %f %f\n", (*pvecGet).vx, (*pvecGet).vy, (*pvecGet).vz);
	return 1;
    case FAT_BEHIND_GET_SHOOT:
	if ( !(nValidFlag & FAT_BH_VALID_S) ) return 0;
	(*pvecGet) = vdatBehind.vecShoot;
	work->vecCautionPos = vdatBehind.vecPos;
//printf("shot = %f %f %f\n", (*pvecGet).vx, (*pvecGet).vy, (*pvecGet).vz);
	return 1;
    case FAT_BEHIND_GET_HARD:
	if ( !(nValidFlag & FAT_BH_VALID_H) ) return 0;	
	(*pvecGet) = vdatBehind.vecHard;
	work->vecCautionPos = vdatBehind.vecPos;
//printf("herd = %f %f %f\n", (*pvecGet).vx, (*pvecGet).vy, (*pvecGet).vz);
	return 1;
    }
    return 0;
}
static void FAT_DbgViewBehindPos(FAT_BEHIND_UNIT*	unit)
{
    int i, nBoundRes;
    for ( i = 0; i < unit->nDataNum; i++ ){
	nBoundRes = FvecBoundCheckQuad( &unit->vdatArea[i].vecLTop, &unit->vdatArea[i].vecRBottom, 
					&unit->vdatArea[i].vecCenter, &GM_PlayerPosition);
	if ( nBoundRes != 0){ // 内包された
	    if ( unit->nType[i] & FAT_BEHIND_DIR_UD )
		NewBoundingBoxView( &unit->vdatPos[i].vecLTop, &unit->vdatPos[i].vecRBottom, 0, 0, 255);
	    else
		NewBoundingBoxView( &unit->vdatPos[i].vecLTop, &unit->vdatPos[i].vecRBottom, 255, 0, 0);
	}else{
	    if ( unit->nType[i] & FAT_BEHIND_DIR_UD )
		NewBoundingBoxView( &unit->vdatPos[i].vecLTop, &unit->vdatPos[i].vecRBottom, 0, 0, 64);
	    else
		NewBoundingBoxView( &unit->vdatPos[i].vecLTop, &unit->vdatPos[i].vecRBottom, 64, 0, 0);
	}
    }

}

static void FAT_DbgViewBehindArea(FAT_BEHIND_UNIT*	unit)
{
    int i, nBoundRes;
    for ( i = 0; i < unit->nDataNum; i++ ){
	nBoundRes = FvecBoundCheckQuad( &unit->vdatArea[i].vecLTop, &unit->vdatArea[i].vecRBottom, 
					&unit->vdatArea[i].vecCenter, &GM_PlayerPosition);
	if ( nBoundRes != 0){ // 内包された
	    if ( unit->nType[i] & FAT_BEHIND_DIR_UD )
		NewBoundingBoxView( &unit->vdatArea[i].vecLTop, &unit->vdatArea[i].vecRBottom, 0, 0, 255);
	    else
		NewBoundingBoxView( &unit->vdatArea[i].vecLTop, &unit->vdatArea[i].vecRBottom, 255, 0, 0);
	}else{
	    if ( unit->nType[i] & FAT_BEHIND_DIR_UD )
		NewBoundingBoxView( &unit->vdatArea[i].vecLTop, &unit->vdatArea[i].vecRBottom, 0, 0, 64);
	    else
		NewBoundingBoxView( &unit->vdatArea[i].vecLTop, &unit->vdatArea[i].vecRBottom, 64, 0, 0);
	}
    }

}

// 指定点が指定ボックスに入っているかチェック
// -1 		  : 失敗
// インデックス値 : 成功
static int FAT_CheckPointBehindBox(FAT_BEHIND_UNIT*	unit,		// ビハインドワーク
				   FVECTOR 		vecPos,		// 指定点
				   FAT_BEHIND_VECDATA* 	pvdatBehind )   // ビハインドデータが入る
{
    int i;
    int nLRCheck;
    int nUDCheck;
    int nLRIndex;
    int nUDIndex;
    int nLRDist;
    int nUDDist;
    int nBoundRes;
    FAT_BEHIND_VECDATA vdatLR;
    FAT_BEHIND_VECDATA vdatUD;

    nLRCheck = 0;
    nUDCheck = 0;
    nLRIndex = -1;
    nUDIndex = -1;

    for ( i = 0; i < unit->nDataNum; i++ ){
	if ( !nLRCheck && (unit->nType[i] & FAT_BEHIND_DIR_LR) ) { // 横方向チェック
	    nBoundRes = FvecBoundCheckQuad( &unit->vdatArea[i].vecLTop, &unit->vdatArea[i].vecRBottom, 
					    &unit->vdatArea[i].vecCenter, &vecPos);
	    if ( nBoundRes != 0){ // 内包された
		// ビハインド用データ取得
		FAT_GetBehindData( nBoundRes, FAT_BEHIND_DIR_LR, &unit->vdatPos[i], &vdatLR);
		nLRIndex = i;
		nLRCheck = 1; // チェックフラグ
	    }
	}
	if ( !nUDCheck && (unit->nType[i] & FAT_BEHIND_DIR_UD) ) { // 縦方向チェック
	    nBoundRes = FvecBoundCheckQuad( &unit->vdatArea[i].vecLTop, &unit->vdatArea[i].vecRBottom, 
					    &unit->vdatArea[i].vecCenter, &vecPos );
	    if ( nBoundRes != 0){ // 内包された
		// ビハインド用データ取得
		FAT_GetBehindData( nBoundRes, FAT_BEHIND_DIR_UD, &unit->vdatPos[i], &vdatUD);
		nUDIndex = i;
		nUDCheck = 1; // チェックフラグ
	    }
	}
    }

    if ( nLRCheck && nUDCheck){ // 双方内包
	MAO_GetDiffVec3( NULL, &nLRDist, NULL, &vecPos, &vdatLR.vecPos);
	MAO_GetDiffVec3( NULL, &nUDDist, NULL, &vecPos, &vdatUD.vecPos);

	if ( nLRDist < nUDDist ) {
	    (*pvdatBehind) = vdatLR;
	    return (nLRIndex);
	}else{
	    (*pvdatBehind) = vdatUD;
	    return (nUDIndex);
	}
    }else if ( !nLRCheck && !nUDCheck){ // 双方内包されなかった
	return (-1);
    }else { // 片方に内包
	if ( nLRCheck ){
	    (*pvdatBehind) = vdatLR;
	    return (nLRIndex);
	}else{
	    (*pvdatBehind) = vdatUD;
	    return (nUDIndex);
	}
    }

    return (-1); // 失敗
}
// 指定点に対応した指定形式の座標をビハインドデータから取得
static int FAT_GetPointFromBehind( Work* work, FVECTOR* pvecGet, FVECTOR vecPos, int nGetFlag )
{
    int nIndex;
    int nValidFlag;
    FAT_BEHIND_UNIT* unit;
    FAT_BEHIND_VECDATA vdatBehind;	

    unit = &work->unitBehind;

    if ( (nIndex = FAT_CheckPointBehindBox( unit, vecPos, &vdatBehind)) == -1){
	return 0; // 失敗
    }

    // 有効フラグ取得
    nValidFlag = ( unit->nType[nIndex] & FAT_BH_VALID_CHECK );

    switch (nGetFlag){
    case FAT_BEHIND_GET_BACK:
	if ( !(nValidFlag & FAT_BH_VALID_B) ) return 0;
	(*pvecGet) = vdatBehind.vecBack;
	work->vecCautionPos = vdatBehind.vecPos;
//printf("back = %f %f %f\n", (*pvecGet).vx, (*pvecGet).vy, (*pvecGet).vz);
	return 1;
    case FAT_BEHIND_GET_CORNER:
	if ( !(nValidFlag & FAT_BH_VALID_C) ) return 0;
	(*pvecGet) = vdatBehind.vecCorner;
	work->vecCautionPos = vdatBehind.vecPos;
//printf("corn = %f %f %f\n", (*pvecGet).vx, (*pvecGet).vy, (*pvecGet).vz);
	return 1;
    case FAT_BEHIND_GET_SHOOT:
	if ( !(nValidFlag & FAT_BH_VALID_S) ) return 0;
	(*pvecGet) = vdatBehind.vecShoot;
	work->vecCautionPos = vdatBehind.vecPos;
//printf("shot = %f %f %f\n", (*pvecGet).vx, (*pvecGet).vy, (*pvecGet).vz);
	return 1;
    case FAT_BEHIND_GET_HARD:
	if ( !(nValidFlag & FAT_BH_VALID_H) ) return 0;	
	(*pvecGet) = vdatBehind.vecHard;
	work->vecCautionPos = vdatBehind.vecPos;
//printf("herd = %f %f %f\n", (*pvecGet).vx, (*pvecGet).vy, (*pvecGet).vz);
	return 1;
    }
    return 0;
}
#if 0
// ２種類の対ビハインド点を取得する
// 0 : 失敗
// 1 : 成功
static int FAT_GetPos2FromBehind( Work*    work,	// ファットマンワーク
				  int*	   pnZone,	// 出力:一つ目のゾーン番号
				  int*	   pnZone2,	// 出力:二つ目のゾーン番号
				  FVECTOR* pvecGet,	// 出力:一つ目の位置
				  FVECTOR* pvecGet2,	// 出力:二つ目の位置
				  FVECTOR  vecPos, 	// 位置
				  int 	   nGetFlag,	// 一つ目の取得する位置フラグ
				  int 	   nGetFlag2 )	// 二つ目の取得する位置フラグ
{
    int nRes1, nRes2;

    // 一つ目の点を取得
    if ( (nRes1 = FAT_GetPointFromBehind( work, pvecGet, vecPos, nGetFlag )) != -1){
	(*pnZone) = nRes1;
    }else{
	return 0;
    }
    // 二つ目の点を取得
    if ( (nRes2 = FAT_GetPointFromBehind( work, pvecGet2, vecPos, nGetFlag2 )) != -1){
	(*pnZone2) = nRes2;
    }else{
	return 0;
    }

    return (1);
}
#endif







