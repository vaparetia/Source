/*
    fatintrude.c
    ファットマン戦イントルードチェック
    2001/04/03 Masafumi Okuta
    $id$
*/
enum { // イントルードエリア
FAT_INTRUDE_11,
FAT_INTRUDE_12,
FAT_INTRUDE_13,
FAT_INTRUDE_21,
FAT_INTRUDE_22,
FAT_INTRUDE_31,
FAT_INTRUDE_32,
FAT_INTRUDE_33,
FAT_INTRUDE_41,
FAT_INTRUDE_42,
FAT_INTRUDE_43,
FAT_INTRUDE_MAX,
};
// イントルードエリア判別用
static FVECTOR FAT_IntrudeArea[FAT_INTRUDE_MAX][2] = 
{
    {{44901, 12500, -105460}, { 49111, 12500, -102035}},
    {{44901, 12500, -100420}, { 49111, 12500,  -96383}},
    {{44901, 12500,  -95300}, { 49111, 12500,  -91477}},

    {{50700, 12500, -100420}, { 54965, 12500,  -96383}},
    {{50700, 12500,  -95300}, { 54965, 12500,  -91477}},

    {{58000, 12500, -100420}, { 62500, 12500,  -96383}},
    {{58000, 12500,  -95300}, { 62500, 12500,  -91477}},
    {{58000, 12500,  -90500}, { 62500, 12500,  -87000}},

    {{64000, 12500, -100420}, { 68300, 12500,  -96383}},
    {{64000, 12500,  -95300}, { 68300, 12500,  -91477}},
    {{64000, 12500,  -90500}, { 68300, 12500,  -87000}},
};

// プレイヤーの位置からイントルードエリアを取得
// 成功:エリア番号
// 失敗:-1
static int FAT_GetIntrudeArea( void )
{
    int i;
    FVECTOR vecPla;

    // イントルードチェック
    if ( !(GM_PlayerStatus & PLAYER_INTRUDE) )
	return (-1);

    // プレイヤー位置
    vecPla = GM_PlayerPosition;

    // 内包チェック:高さはややこしいので省く
    for ( i = 0; i < FAT_INTRUDE_MAX; i++){
	if (vecPla.vx >= FAT_IntrudeArea[i][0].vx && 
	    vecPla.vz >= FAT_IntrudeArea[i][0].vz && 
	    vecPla.vx <= FAT_IntrudeArea[i][1].vx && 
	    vecPla.vz <= FAT_IntrudeArea[i][1].vz){
	    return (i);
	}
    }

    return (-1);
}

// ファットマン退避ポイントデータ
enum{
FAT_INTESC_ZONE_1,
FAT_INTESC_ZONE_2,
FAT_INTESC_ZONE_3,
FAT_INTESC_ZONE_4,
FAT_INTESC_ZONE_5,
FAT_INTESC_ZONE_6,
FAT_INTESC_ZONE_7,
FAT_INTESC_ZONE_8,
};

static FVECTOR FAT_IntrudeEscPos[FAT_INTRUDE_ADDR_NUM] = 
{
    { 46896, 12500, -109248 }, 
    { 51728, 12500, -103974 },
    { 69037, 12500, -102472 },

    { 43886, 12500, -86514 },
    { 48910, 12500, -80562 },

    { 51074, 12500, -79800 },
    { 63000, 12500, -80633 },
    { 69853, 12500, -82186 },
};

// 初期化 pos data -> zone data
static void FAT_InitEscPos2Zone( Work* work )
{
    int i;

    for ( i = 0; i < FAT_INTRUDE_ADDR_NUM; i++){
	work->nIntrudeZone[i] = HZX_Zone1( HZX_GetAddress( work->npc.ctrl->hzx_id, &FAT_IntrudeEscPos[i], -1) );
ASSERT(work->nIntrudeZone[i] != -1);
    }
}

// ファットマンの退避アドレスを取得
static int FAT_GetIntrudeEscZone( Work* work, int nIntrudeZone )
{
    int nRes;

    nRes = HZX_NO_ZONE;

    switch ( nIntrudeZone ){
    case FAT_INTRUDE_11:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_7];
	break;
    case FAT_INTRUDE_12:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_8];
	break;
    case FAT_INTRUDE_13:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_3];	
	break;
    case FAT_INTRUDE_21:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_1];	
	break;
    case FAT_INTRUDE_22:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_2];	
	break;
    case FAT_INTRUDE_31:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_1];	
	break;
    case FAT_INTRUDE_32:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_5];	
	break;
    case FAT_INTRUDE_33:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_6];	
	break;
    case FAT_INTRUDE_41:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_5];	
	break;
    case FAT_INTRUDE_42:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_4];	
	break;
    case FAT_INTRUDE_43:
	nRes = work->nIntrudeZone[FAT_INTESC_ZONE_1];	
	break;
    }

    return (nRes);
}

