/*
    fattobidasi.c
    ファットマン思考:コンテナtoコンテナ飛び出し
    2000/04/04 Masafumi Okuta
    $Id: fattobidasi.c,v 1.1.1.3 2002/11/19 11:48:01 Yoshizawa1 Exp $
*/
#define CONTENA_TATE1_Z (-100000.f)
#define CONTENA_TATE2_Z  (-96250.f)
#define CONTENA_TATE3_Z  (-91000.f)
#define CONTENA_TATE4_Z  (-85000.f)


// インデックス選択方法
enum{
FAT_C4_INDEX_FOR,  // 順送り
FAT_C4_INDEX_BACK, // 逆順
FAT_C4_INDEX_DIST, // 遠距離優先
FAT_C4_INDEX_AREA, // 対エリア優先
FAT_C4_INDEX_MAX,	
};

#define FAT_TOBI_YOKO_NUM (15)	// コンテナtoコンテナ飛び出しデータ横
#define FAT_TOBI_TATE_NUM (14)	// コンテナtoコンテナ飛び出しデータ縦
#define FAT_TOBI_TOTAL_NUM (FAT_TOBI_YOKO_NUM+FAT_TOBI_TATE_NUM) // コンテナtoコンテナ飛び出しデータトータル
enum{
FAT_TOBI_Y_00,
FAT_TOBI_Y_01,
FAT_TOBI_Y_02,
FAT_TOBI_Y_03,
FAT_TOBI_Y_04,
FAT_TOBI_Y_05,
FAT_TOBI_Y_06,
FAT_TOBI_Y_07,
FAT_TOBI_Y_08,
FAT_TOBI_Y_09,
FAT_TOBI_Y_10,
FAT_TOBI_Y_11,
FAT_TOBI_Y_12,
FAT_TOBI_Y_13,
FAT_TOBI_Y_14,
FAT_TOBI_T_00,
FAT_TOBI_T_01,
FAT_TOBI_T_02,
FAT_TOBI_T_03,
FAT_TOBI_T_04,
FAT_TOBI_T_05,
FAT_TOBI_T_06,
FAT_TOBI_T_07,
FAT_TOBI_T_08,
FAT_TOBI_T_09,
FAT_TOBI_T_10,
FAT_TOBI_T_11,
FAT_TOBI_T_12,
FAT_TOBI_T_13,
};


// コンテナtoコンテナ飛び出しデータ横
static FVECTOR FAT_TobidasiYokoPos[FAT_TOBI_YOKO_NUM] = { 
{ 47038.f, 12426.f, -106803.f }, // 左上コンテナ 上
{ 46681.f, 12426.f, -101213.f }, // コンテナライン１
{ 52780.f, 12426.f, -101301.f }, // 
{ 60217.f, 12426.f, -101167.f }, // 
{ 66311.f, 12426.f, -100986.f }, // 
{ 47028.f, 12426.f, -95903.f }, //  コンテナライン２
{ 52786.f, 12559.f, -96162.f }, // 
{ 59734.f, 12496.f, -95577.f }, // 
{ 65706.f, 12554.f, -96183.f }, // 
{ 46836.f, 12537.f, -90531.f }, //  コンテナライン３
{ 52690.f, 12519.f, -90848.f }, // 
{ 60018.f, 12563.f, -90876.f }, // 
{ 65633.f, 12510.f, -91198.f }, // 
{ 60283.f, 12500.f, -85912.f }, // コンテナ 下
{ 66101.f, 12559.f, -85847.f }, //  
};

// コンテナtoコンテナ飛び出しデータ縦 左上から右下へ
static FVECTOR FAT_TobidasiTatePos[FAT_TOBI_TATE_NUM] = {  
{ 43643.f, 12426.f, -103630.f }, // 左端
{ 43672.f, 12502.f, -98374.f }, // 
{ 43865.f, 12426.f, -93438.f }, // 
{ 50006.f, 12426.f, -98654.f }, // 左
{ 50042.f, 12426.f, -93362.f }, // 
{ 56432.f, 12426.f, -98388.f }, // 中心
{ 56750.f, 12426.f, -93654.f }, // 
{ 56655.f, 12426.f, -88401.f }, // 
{ 63266.f, 12426.f, -98463.f }, // 右
{ 63073.f, 12426.f, -93446.f }, // 
{ 62973.f, 12426.f, -88452.f }, // 
{ 68824.f, 12426.f, -98602.f }, // 右端
{ 69148.f, 12490.f, -93296.f }, // 
{ 68894.f, 12563.f, -88909.f }, // 
};


static int FAT_GetVectorScalar(FVECTOR vec1, FVECTOR vec2)
{
    FVECTOR vecTmp;

    _sceVu0SubVector( &vecTmp, &vec1, &vec2 );

    return ( _FVecLen3( &vecTmp )); 
}

// コンテナtoコンテナ飛び出し用位置データからゾーン取得
static void FAT_InitTobidasiPos2Zone( Work* work )
{
    int i,j;

    for ( i = 0; i < FAT_TOBI_YOKO_NUM; i++){
	work->nTobidasiZone[i] = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &FAT_TobidasiYokoPos[i], -1) );
    }

    for ( i = FAT_TOBI_YOKO_NUM, j = 0; i < FAT_TOBI_TOTAL_NUM; i++, j++){
	work->nTobidasiZone[i] = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &FAT_TobidasiTatePos[j], -1) );
    }
}

// 対飛び出しゾーンインデックス作成
static void  FAT_InitContTobiIndex( Work* work )
{
    int i,j;
    int nResIndex;
    int nTmp, nNear;

    // 横方向検索
    for ( i = 0; i < FAT_TOBI_YOKO_NUM; i++){
	nNear = 120000;
	nResIndex = 0;
	for ( j = 0; j < FAT_TOBI_TATE_NUM; j++){
	    nTmp = FAT_GetVectorScalar( FAT_TobidasiYokoPos[i], FAT_TobidasiTatePos[j]);
	    if ( nTmp < nNear ){
		nResIndex = j + FAT_TOBI_YOKO_NUM;
		nNear = nTmp;
	    }
	}
	work->nContTobiIndex[i] = nResIndex;
    }

    // 縦方向検索
    for ( i = 0; i < FAT_TOBI_TATE_NUM; i++){
	nNear = 120000;
	nResIndex = 0;
	for ( j = 0; j < FAT_TOBI_YOKO_NUM; j++){
	    nTmp = FAT_GetVectorScalar( FAT_TobidasiTatePos[i], FAT_TobidasiYokoPos[j]);
	    if ( nTmp < nNear ){
		nResIndex = j;
		nNear = nTmp;
	    }
	}
	work->nContTobiIndex[i + FAT_TOBI_YOKO_NUM] = nResIndex;
    }
}


// 左上
static int TobiZone_00( Work* work )
{  
    int nRes;

    if ( GM_PlayerPosition.vx < 48000.f){
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_04;	// 
	else 			    			   nRes = FAT_TOBI_Y_01;	// 
    }else if ( GM_PlayerPosition.vx < 52250.f){
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_T_00;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_02;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_06;	// 
	else 			    			   nRes = FAT_TOBI_Y_05;	// 
    }else{
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z ) 	   nRes = FAT_TOBI_T_00;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_T_01;	// 
	else 			    			   nRes = FAT_TOBI_T_02;	// 
    }

    return nRes; 
}
// 左
static int TobiZone_01( Work* work ){  
    int nRes;

    if ( GM_PlayerPosition.vx < 48000.f){
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_04;	// 
	else 			    			   nRes = FAT_TOBI_Y_05;	// 
    }else if ( GM_PlayerPosition.vx < 52250.f){
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_T_01;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_02;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_06;	// 
	else 			    			   nRes = FAT_TOBI_Y_05;	// 
    }else{
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_00;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_01;	// 
	else 			    			   nRes = FAT_TOBI_T_02;	// 
    }

    return nRes; 

}
// コンテナ : 上側(2-2)
static int TobiZone_02( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vx < 48000.f){	// 左
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_05;	// 
	else 			    			   nRes = FAT_TOBI_Y_04;	// 
    }else if ( GM_PlayerPosition.vx < 52250.f){	// 左側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_06;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_01;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_05;	// 
	else 			    			   nRes = FAT_TOBI_Y_06;	// 
    }else if ( GM_PlayerPosition.vx < 58850.f){ // 右側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_07;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_03;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_07;	// 
	else 			    			   nRes = FAT_TOBI_Y_06;	// 
    }else{					// 右
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_05;	// 
	else 			    			   nRes = FAT_TOBI_T_06;	// 
    }

    return nRes; 
}
static int TobiZone_03( Work* work )
{ 
    int nRes;

    if ( GM_PlayerPosition.vx < 52250.f){	// 左
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_06;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_01;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_05;	// 
	else 			    			   nRes = FAT_TOBI_Y_06;	// 
    }else if ( GM_PlayerPosition.vx < 58850.f){ // 左側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_07;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_03;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_07;	// 
	else 			    			   nRes = FAT_TOBI_Y_06;	// 
    }else if ( GM_PlayerPosition.vx < 60000.f){ // 右側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_02;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_06;	// 
	else 			    			   nRes = FAT_TOBI_Y_07;	// 
    }else{					// 右
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_T_09;	// 
	else 			    			   nRes = FAT_TOBI_T_10;	// 
    }

    return nRes; 
}
static int TobiZone_04( Work* work )
{ 
    int nRes;

    if ( GM_PlayerPosition.vx < 60500.f){ // 左側
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_09;	// 
	else 			    			   nRes = FAT_TOBI_T_10;	// 
    }else if ( GM_PlayerPosition.vx < 64250.f){
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z ) 	   nRes = FAT_TOBI_Y_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_03;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_07;	// 
	else 			    			   nRes = FAT_TOBI_Y_11;	// 
    }else{
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_09;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE4_Z ) nRes = FAT_TOBI_Y_12;	// 
	else 			    			   nRes = FAT_TOBI_T_10;	// 
    }

    return nRes; 
}
// コンテナ 3-1上部
static int TobiZone_05( Work* work )
{ 
    int nRes;

    if ( GM_PlayerPosition.vx < 48000.f){ // 左端
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_04;	// 
	else 			    			   nRes = FAT_TOBI_Y_05;	// 
    }else if ( GM_PlayerPosition.vx < 52250.f){	// 右側通路
	nRes = FAT_TOBI_Y_06;	// 
    }else  if ( GM_PlayerPosition.vx < 58850.f){ // 右
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_03;	// 
	else 			    			   nRes = FAT_TOBI_T_04;	// 
    }else {	// 離れ過ぎ
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_04;	// 
    }

    return nRes; 
}
// コンテナ 3-2上部
static int TobiZone_06( Work* work )
{  
    int nRes;

    if ( GM_PlayerPosition.vx < 48000.f){	// 左
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_03;	// 
	else 			    			   nRes = FAT_TOBI_T_04;	// 
    }else if ( GM_PlayerPosition.vx < 52250.f){	// 左側通路
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_05;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_09;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_05;	// 
	else 			    			   nRes = FAT_TOBI_Y_09;	// 
    }else if ( GM_PlayerPosition.vx < 58850.f){ // 右側通路
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_07;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_11;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_07;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE4_Z ) nRes = FAT_TOBI_Y_07;	// 
	else 			    			   nRes = FAT_TOBI_Y_14;	// 
    }else if ( GM_PlayerPosition.vx < 64250.f){ // 右
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_05;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_06;	// 
	else 			    			   nRes = FAT_TOBI_T_07;	// 
    }else{	   				// 離れ過ぎ
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_06;	// 
    }

    return nRes; 
}
static int TobiZone_07( Work* work )
{ 
    int nRes;

    if ( GM_PlayerPosition.vx < 52250.f){	// 左
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_05;	// 
	else 			    			   nRes = FAT_TOBI_T_06;	// 
    }else if ( GM_PlayerPosition.vx < 58850.f){ // 左側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_06;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_11;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_06;	// 
	else 			    			   nRes = FAT_TOBI_Y_10;	// 
    }else if ( GM_PlayerPosition.vx < 60000.f){ // 右側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_11;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_08;	// 
	else 			    			   nRes = FAT_TOBI_Y_12;	// 
    }else if ( GM_PlayerPosition.vx < 64250.f){	// 右
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_T_09;	// 
	else 			    			   nRes = FAT_TOBI_T_10;	// 
    }else {					// 離れ過ぎ
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_04;	// 
    }

    return nRes; 
}
static int TobiZone_08( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vx < 58850.f){ // 左 
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vx < 60500.f){ // 左側
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_09;	// 
	else 			    			   nRes = FAT_TOBI_T_10;	// 
    }else if ( GM_PlayerPosition.vx < 64250.f){
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z ) 	   nRes = FAT_TOBI_Y_07;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_12;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_07;	// 
	else 			    			   nRes = FAT_TOBI_Y_11;	// 
    }else{
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_09;	// 
	else						   nRes = FAT_TOBI_T_10;	// 
    }

    return nRes; 
}
static int TobiZone_09( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vx < 48000.f){ // 左端
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_04;	// 
	else 			    			   nRes = FAT_TOBI_Y_05;	// 
    }else if ( GM_PlayerPosition.vx < 52250.f){	// 右側通路
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z )	   nRes = FAT_TOBI_Y_06;	// 
	else						   nRes = FAT_TOBI_Y_10;	// 
    }else  if ( GM_PlayerPosition.vx < 58850.f){ // 右
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_03;	// 
	else 			    			   nRes = FAT_TOBI_T_04;	// 
    }else {	// 離れ過ぎ
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_04;	// 
    }

    return nRes; 
}
static int TobiZone_10( Work* work )
{ 
    int nRes;

    if ( GM_PlayerPosition.vx < 48000.f){	// 左
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_03;	// 
	else 			    			   nRes = FAT_TOBI_T_04;	// 
    }else if ( GM_PlayerPosition.vx < 52250.f){	// 左側通路
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_05;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_09;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_09;	// 
	else 			    			   nRes = FAT_TOBI_Y_06;	// 
    }else if ( GM_PlayerPosition.vx < 58850.f){ // 右側通路
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_07;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_11;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_11;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE4_Z ) nRes = FAT_TOBI_Y_11;	// 
	else 			    			   nRes = FAT_TOBI_Y_14;	// 
    }else if ( GM_PlayerPosition.vx < 64250.f){ // 右
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_05;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_06;	// 
	else 			    			   nRes = FAT_TOBI_T_07;	// 
    }else{	   				// 離れ過ぎ
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_06;	// 
    }

    return nRes; 
}
static int TobiZone_11( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vx < 48000.f){	// 離れ過ぎ
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_06;	// 
    }else if ( GM_PlayerPosition.vx < 52250.f){	// 左
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_05;	// 
	else 			    			   nRes = FAT_TOBI_T_06;	// 
    }else if ( GM_PlayerPosition.vx < 58850.f){ // 左側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_07;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_13;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_07;	// 
	else 			    			   nRes = FAT_TOBI_Y_14;	// 
    }else if ( GM_PlayerPosition.vx < 60000.f){ // 右側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_12;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_13;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_12;	// 
	else 			    			   nRes = FAT_TOBI_Y_14;	// 
    }else{	// 右
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_T_09;	// 
	else 			    			   nRes = FAT_TOBI_T_10;	// 
    }

    return nRes; 
}
static int TobiZone_12( Work* work )
{   
    int nRes;

    if ( GM_PlayerPosition.vx < 58850.f){ // 左 
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vx < 60500.f){ // 左側
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_09;	// 
	else 			    			   nRes = FAT_TOBI_T_10;	// 
    }else if ( GM_PlayerPosition.vx < 64250.f){
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z ) 	   nRes = FAT_TOBI_Y_11;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_14;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_11;	// 
	else 			    			   nRes = FAT_TOBI_Y_14;	// 
    }else{
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_09;	// 
	else						   nRes = FAT_TOBI_T_10;	// 
    }

    return nRes; 
}
static int TobiZone_13( Work* work )
{   
    int nRes;

    if ( GM_PlayerPosition.vx < 48000.f){	// 離れ過ぎ
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_06;	// 
    }else if ( GM_PlayerPosition.vx < 52250.f){	// 左
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_05;	// 
	else 			    			   nRes = FAT_TOBI_T_06;	// 
    }else if ( GM_PlayerPosition.vx < 58850.f){ // 左側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_06;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_11;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_06;	// 
	else 			    			   nRes = FAT_TOBI_Y_10;	// 
    }else if ( GM_PlayerPosition.vx < 60000.f){ // 右側
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z )	   nRes = FAT_TOBI_Y_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_11;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_08;	// 
	else 			    			   nRes = FAT_TOBI_Y_12;	// 
    }else{	// 右
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_T_09;	// 
	else 			    			   nRes = FAT_TOBI_T_10;	// 
    }

    return nRes; 
}
static int TobiZone_14( Work* work )
{ 
    int nRes;

    if ( GM_PlayerPosition.vx < 58850.f){ // 左 
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vx < 60500.f){ // 左側
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_09;	// 
	else 			    			   nRes = FAT_TOBI_T_10;	// 
    }else if ( GM_PlayerPosition.vx < 64250.f){
	if ( GM_PlayerPosition.vz < CONTENA_TATE1_Z ) 	   nRes = FAT_TOBI_Y_07;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) nRes = FAT_TOBI_Y_12;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_Y_07;	// 
	else 			    			   nRes = FAT_TOBI_Y_11;	// 
    }else{
	if ( GM_PlayerPosition.vz < CONTENA_TATE2_Z ) 	   nRes = FAT_TOBI_T_08;	// 
	else if ( GM_PlayerPosition.vz < CONTENA_TATE3_Z ) nRes = FAT_TOBI_T_09;	// 
	else						   nRes = FAT_TOBI_T_10;	// 
    }

    return nRes; 
}
#define CONTENA_YOKO1_Z (-103000.f)
#define CONTENA_YOKO2_Z (-98400.f)
#define CONTENA_YOKO3_Z (-93250.f)
#define CONTENA_YOKO4_Z (-88500.f)
#define CONTENA_YOKO5_Z (-84000.f)

#define CONTENA_YOKO1_X (45100.f)
#define CONTENA_YOKO2_X (48500.f)
#define CONTENA_YOKO3_X (51000.f)
#define CONTENA_YOKO4_X (54330.f)
#define CONTENA_YOKO5_X (58100.f)
#define CONTENA_YOKO6_X (61500.f)
#define CONTENA_YOKO7_X (64100.f)
#define CONTENA_YOKO8_X (68000.f)


// コンテナ1-1 左側
static int TobiZone_15( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)	    nRes = FAT_TOBI_T_01;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)	    nRes = FAT_TOBI_T_02;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)	    nRes = FAT_TOBI_T_02;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_16( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)	    nRes = FAT_TOBI_Y_01;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)	    nRes = FAT_TOBI_T_02;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)	    nRes = FAT_TOBI_T_02;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_17( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)	    nRes = FAT_TOBI_T_01;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)	    nRes = FAT_TOBI_Y_05;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)	    nRes = FAT_TOBI_Y_05;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
// 2-2
static int TobiZone_18( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)        nRes = FAT_TOBI_T_02;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)        nRes = FAT_TOBI_T_02;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   	    nRes = FAT_TOBI_T_02;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_19( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   	    nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)        nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_02;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO3_X)        nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_02;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_20( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   	    nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   	    nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   	    nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_21( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_07;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_22( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_23( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_11;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_24( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_12;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_12;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_25( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_26( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_27( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}
static int TobiZone_28( Work* work )
{
    int nRes;

    if ( GM_PlayerPosition.vz < CONTENA_YOKO1_Z){ 	// 上
	nRes = FAT_TOBI_Y_00;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO2_Z){ // コンテナ１ 
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_03;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_05;	// 
	else 						    nRes = FAT_TOBI_T_08;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO3_Z){ // コンテナ２
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else if ( GM_PlayerPosition.vz < CONTENA_YOKO4_Z){ // コンテナ３
	if ( GM_PlayerPosition.vx < CONTENA_YOKO5_X)   nRes = FAT_TOBI_T_04;	// 
	else if ( GM_PlayerPosition.vx < CONTENA_YOKO7_X)   nRes = FAT_TOBI_T_06;	// 
	else 						    nRes = FAT_TOBI_T_09;	// 
    }else {						// 下
	if ( work->nPlaTobiIndex != -1)	    nRes = work->nContTobiIndex[work->nPlaTobiIndex];
	else				    nRes = FAT_TOBI_Y_00;	// 
    }

    return nRes;
}


// 飛び出しゾーンの目標ゾーンを出す
static int FAT_GetTobidasiAimZone(Work* work, int* pnResIndex)
{
    int nResZone;
    int nResIndex;
    
    // 関数テーブル
    int ( *tobizone_check[FAT_TOBI_TOTAL_NUM] )(Work* ) = {
	TobiZone_00, 	TobiZone_01, 	TobiZone_02,	TobiZone_03, 
	TobiZone_04, 	TobiZone_05,	TobiZone_06,	TobiZone_07,
	TobiZone_08,	TobiZone_09,	TobiZone_10,	TobiZone_11,
	TobiZone_12,	TobiZone_13,	TobiZone_14,	TobiZone_15,
	TobiZone_16,	TobiZone_17,	TobiZone_18,	TobiZone_19,
	TobiZone_20,	TobiZone_21,	TobiZone_22,	TobiZone_23,
	TobiZone_24,	TobiZone_25,	TobiZone_26,	TobiZone_27,
	TobiZone_28
    };
    
    nResZone = -1;
    
    // 関数呼出
    if ( work->nFatTobiIndex != -1 ){
	nResIndex = ( *tobizone_check[work->nFatTobiIndex] )( work );
	if ( pnResIndex != NULL ){
	    *pnResIndex = nResIndex;
	}
	nResZone = work->nTobidasiZone[ nResIndex ];
    }


    return (nResZone);
}

// 現在ファットマンとプレイヤーがいる飛び出しゾーンを設定
static void FAT_SetTobidasiArea( Work* work )
{
    NPCWORK* npc;
    int i;

    npc = &work->npc;

    work->nFatTobiIndex = -1;
    work->nPlaTobiIndex = -1;

    for ( i = 0; i < FAT_TOBI_TOTAL_NUM; i++){
	if ( HZX_Zone1( npc->ctrl->addr ) == work->nTobidasiZone[i] ){
	    work->nFatTobiIndex = i;
	}
	if ( HZX_Zone1( GM_PlayerControl->addr ) == work->nTobidasiZone[i] ){
	    work->nPlaTobiIndex = i;
	}
    }
}
