/*
    fatarea.c
    ファットマン戦用エリアシステム
    2001/03/19 Masafumi Okuta
    $Id: fatarea.c,v 1.1.1.3 2002/11/19 11:47:56 Yoshizawa1 Exp $
*/

// エリア対応座標データからゾーン番号を取得
static void FAT_InitAreaPos2Zone( Work* work )
{
    int i,j;

    for ( i = 0; i < FATC4_AREA_MAX; i++ ){
	for ( j = 0; j < 5; j++ ){
	    work->nAreaZone[i][j] = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &FAT_AREADATA_POS[i][j], -1) );
	    ASSERT(work->nAreaZone[i][j] != -1);
	}
    }
}


// 位置からエリア番号を返す
static int FAT_GetArea(FVECTOR *vecPos)
{
    if ( vecPos->vz < -100000.f ){ 	// ヘリポートエリア
	if (vecPos->vx < 54000.f){
	    return FATC4_AREA_HERI_LEFT;
	}else if (vecPos->vx < 64000.f){
	    return FATC4_AREA_HERI_CENTER;
	}else{
	    return FATC4_AREA_HERI_RIGHT;
	}	
    }else if ( vecPos->vz < -90500.f ){ 	// コンテナエリア
	if (vecPos->vx < 56500.f){
	    return FATC4_AREA_CONT_LEFT;
	}else{
	    return FATC4_AREA_CONT_RIGHT;
	}
    }else {
	if (vecPos->vx < 56500.f){		// 棚エリア
	    return FATC4_AREA_BANQ_LEFT;
	}else{
	    return FATC4_AREA_BANQ_RIGHT;
	}
    }
}
// 対となるエリアを返す
static int FAT_GetContArea( int nArea)
{
    int nSeed;

    nSeed = KR_RandU( 10 );

    switch(nArea){
    case FATC4_AREA_HERI_LEFT:	// ヘリポート左エリア
	if (nSeed < 3)         return FATC4_AREA_CONT_RIGHT;
	else if (nSeed < 6)    return FATC4_AREA_BANQ_LEFT;
	else		       return FATC4_AREA_BANQ_RIGHT;
	break;
    case FATC4_AREA_HERI_CENTER: // ヘリポート中央エリア
	if (nSeed < 5)         return FATC4_AREA_BANQ_LEFT;
	else		       return FATC4_AREA_BANQ_RIGHT;
	break;
    case FATC4_AREA_HERI_RIGHT: // ヘリポート右エリア
	if (nSeed < 3)         return FATC4_AREA_CONT_LEFT;
	else if (nSeed < 6)    return FATC4_AREA_BANQ_LEFT;
	else		       return FATC4_AREA_BANQ_RIGHT;
	break;
    case FATC4_AREA_CONT_LEFT: // コンテナエリア左半分
	if (nSeed < 1)         return FATC4_AREA_HERI_CENTER;
	else if (nSeed < 4)    return FATC4_AREA_HERI_RIGHT;
	else if (nSeed < 7)    return FATC4_AREA_BANQ_LEFT;
	else		       return FATC4_AREA_BANQ_RIGHT;
	break;
    case FATC4_AREA_CONT_RIGHT:	// コンテナエリア右半分
	if (nSeed < 1)         return FATC4_AREA_HERI_CENTER;
	else if (nSeed < 4)    return FATC4_AREA_HERI_RIGHT;
	else if (nSeed < 7)    return FATC4_AREA_BANQ_LEFT;
	else		       return FATC4_AREA_BANQ_RIGHT;
	break;
    case FATC4_AREA_BANQ_LEFT:	// 棚エリア左半分
	if (nSeed < 1)         return FATC4_AREA_HERI_LEFT;
	else if (nSeed < 4)    return FATC4_AREA_HERI_CENTER;
	else if (nSeed < 7)    return FATC4_AREA_HERI_RIGHT;
	else		       return FATC4_AREA_CONT_RIGHT;
	break;
    case FATC4_AREA_BANQ_RIGHT:	// 棚エリア右半分
	if (nSeed < 1)         return FATC4_AREA_HERI_LEFT;
	else if (nSeed < 4)    return FATC4_AREA_HERI_CENTER;
	else if (nSeed < 7)    return FATC4_AREA_HERI_RIGHT;
	else		       return FATC4_AREA_CONT_LEFT;
	break;
    }
    return -1;
}
// 対となるエリアをビットフラグ形式で返す
static int FAT_GetContAreaBit( int nArea)
{
    u_int nFlag;
    int   nSeed;

    nSeed = KR_RandU( 10 );

    switch(nArea){
    case FATC4_AREA_HERI_LEFT:	// ヘリポート左エリア
	nFlag =  (1 << FATC4_AREA_CONT_RIGHT);
	nFlag |= (1 << FATC4_AREA_BANQ_LEFT);
	nFlag |= (1 << FATC4_AREA_BANQ_RIGHT);
	break;
    case FATC4_AREA_HERI_CENTER: // ヘリポート中央エリア
	nFlag =  (1 << FATC4_AREA_BANQ_LEFT);
	nFlag |= (1 << FATC4_AREA_BANQ_RIGHT);
	break;
    case FATC4_AREA_HERI_RIGHT: // ヘリポート右エリア
	nFlag =  (1 << FATC4_AREA_CONT_LEFT);
	nFlag |= (1 << FATC4_AREA_BANQ_LEFT);
	nFlag |= (1 << FATC4_AREA_BANQ_RIGHT);
	break;
    case FATC4_AREA_CONT_LEFT: // コンテナエリア左半分
	nFlag =  (1 << FATC4_AREA_HERI_CENTER);
	nFlag |= (1 << FATC4_AREA_HERI_RIGHT);
	nFlag |= (1 << FATC4_AREA_BANQ_LEFT);
	nFlag |= (1 << FATC4_AREA_BANQ_RIGHT);
	break;
    case FATC4_AREA_CONT_RIGHT:	// コンテナエリア右半分
	nFlag =  (1 << FATC4_AREA_HERI_CENTER);
	nFlag |= (1 << FATC4_AREA_HERI_RIGHT);
	nFlag |= (1 << FATC4_AREA_BANQ_LEFT);
	nFlag |= (1 << FATC4_AREA_BANQ_RIGHT);
	break;
    case FATC4_AREA_BANQ_LEFT:	// 棚エリア左半分
	nFlag =  (1 << FATC4_AREA_HERI_LEFT);
	nFlag |= (1 << FATC4_AREA_HERI_CENTER);
	nFlag |= (1 << FATC4_AREA_HERI_RIGHT);
        nFlag |= (1 << FATC4_AREA_CONT_RIGHT);
	break;
    case FATC4_AREA_BANQ_RIGHT:	// 棚エリア右半分
	nFlag =  (1 << FATC4_AREA_HERI_LEFT);
	nFlag |= (1 << FATC4_AREA_HERI_CENTER);
	nFlag |= (1 << FATC4_AREA_HERI_RIGHT);
	nFlag |= (1 << FATC4_AREA_CONT_LEFT);
	break;
    default:
	nFlag = 0;
    }
    return (nFlag);
}

#if 0
// 対となる走り設置エリアを返す
static int FAT_GetContRunPutArea( Work* work, int nArea)
{
    NPCWORK* npc;
    
    npc = &work->npc;

    switch(nArea){
    case FATC4_AREA_HERI_LEFT:	// ヘリポート左エリア
	if ( npc->ctrl->rot.vy < 2048)    return FATC4_AREA_CONT_RIGHT; 	// 右向き
	else			    	   return FATC4_AREA_BANQ_LEFT; 	// 左向き
	break;
    case FATC4_AREA_HERI_CENTER: // ヘリポート中央エリア
	if ( npc->ctrl->rot.vy < 2048)    return FATC4_AREA_BANQ_RIGHT;	// 右向き
	else			    	   return FATC4_AREA_BANQ_LEFT; 	// 左向き
	break;
    case FATC4_AREA_HERI_RIGHT: // ヘリポート右エリア
	if ( npc->ctrl->rot.vy < 2048)    return FATC4_AREA_BANQ_RIGHT;	// 右向き
	else			 	   return FATC4_AREA_BANQ_LEFT; 	// 左向き
	break;
    case FATC4_AREA_CONT_LEFT: // コンテナエリア左半分
	if ( npc->ctrl->rot.vy >= 512 && npc->ctrl->rot.vy < 1536)       return FATC4_AREA_CONT_RIGHT; // 右向き
	else if ( npc->ctrl->rot.vy >= 1536 && npc->ctrl->rot.vy < 2560) return FATC4_AREA_HERI_LEFT;  // 上向き
	else if ( npc->ctrl->rot.vy >= 2560 && npc->ctrl->rot.vy < 3584) return FATC4_AREA_BANQ_LEFT;  // 左向き
	else								   return FATC4_AREA_BANQ_LEFT;  // 下向き
	break;
    case FATC4_AREA_CONT_RIGHT:	// コンテナエリア右半分
	if ( npc->ctrl->rot.vy >= 512 && npc->ctrl->rot.vy < 1536)       return FATC4_AREA_CONT_LEFT;   // 右向き
	else if ( npc->ctrl->rot.vy >= 1536 && npc->ctrl->rot.vy < 2560) return FATC4_AREA_HERI_RIGHT;  // 上向き
	else if ( npc->ctrl->rot.vy >= 2560 && npc->ctrl->rot.vy < 3584) return FATC4_AREA_BANQ_RIGHT;  // 左向き
	else								   return FATC4_AREA_BANQ_RIGHT;  // 下向き
	break;
    case FATC4_AREA_BANQ_LEFT:	// 棚エリア左半分
	if ( npc->ctrl->rot.vy < 2048)    return FATC4_AREA_HERI_RIGHT;	// 右向き
	else			    	   return FATC4_AREA_HERI_LEFT; 	// 左向き
	break;
    case FATC4_AREA_BANQ_RIGHT:	// 棚エリア右半分
	if ( npc->ctrl->rot.vy < 2048)    return FATC4_AREA_HERI_LEFT;		// 右向き
	else			    	   return FATC4_AREA_HERI_RIGHT; 	// 左向き
	break;
    }
    return -1;
}

#endif





