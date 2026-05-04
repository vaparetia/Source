/*
    pardata.c
    オウム関連データ
    2001/05/12 Masafumi Okuta
    $Id: pardata.c,v 1.1.1.3 2002/11/19 11:48:06 Yoshizawa1 Exp $
*/
#ifdef PARROT_USE_CHILD // 子ターゲット使用
static int	Child_Level_Num[] = {  // 子ターゲットレベル
	0,	// Level 0 のターゲット数 
	2,	// Level 1 のターゲット数 
	8,	// Level 2 のターゲット数 
	1	// Level 3 のターゲット数 
};

enum { // 子ターゲット
// Level 1
SMP_TARGET_CHILD_HEAD,  // 頭
SMP_TARGET_CHILD_HART,  // 心臓
// Level 2
SMP_TARGET_CHILD_ARMR1, // 右腕
SMP_TARGET_CHILD_ARMR2, // 
SMP_TARGET_CHILD_ARML1, // 左腕
SMP_TARGET_CHILD_ARML2, // 
SMP_TARGET_CHILD_LEGR1, // 右足
SMP_TARGET_CHILD_LEGR2, // 
SMP_TARGET_CHILD_LEGL1, // 左足
SMP_TARGET_CHILD_LEGL2, // 
// Level 3
SMP_TARGET_CHILD_BPDY,  // 
};

static int Cheild_Target_Connect[] = { // 子ターゲット接続部分
HUMAN21_ATAMA,
HUMAN21_MUNE,

HUMAN21_MIGI_UDE1,
HUMAN21_MIGI_UDE2,
HUMAN21_HIDARI_UDE1,
HUMAN21_HIDARI_UDE2,
HUMAN21_MIGI_ASHI1,
HUMAN21_MIGI_ASHI2,
HUMAN21_HIDARI_ASHI1,
HUMAN21_HIDARI_ASHI2,

HUMAN21_MUNE
};

static FVECTOR	Cheild_Target_Shift[] = { // 子ターゲットシフト値
    // level 0 は無し 
    // level 1 
    { 0.f, 30.f, 0.f },		// 頭 
    { 100.f, 0.f, 100.f },	// 心臓 
    // level 2 
    { 0.f, -130.f, 0.f },	// 右腕１ 
    { 0.f, -130.f, 0.f },	// 右腕２ 
    { 0.f, -130.f, 0.f },	// 左腕１ 
    { 0.f, -130.f, 0.f },	// 左腕２ 
    { -10.f, -200.f, -10.f },	// 右足１ 
    { -20.f, -250.f, -20.f },	// 右足２ 
    { -10.f, -200.f, -10.f },	// 左足１ 
    { -20.f, -250.f, -20.f },	// 左足２ 
    // level 3 
    { 0.f, 0.f, 0.f },		// 胴体 
};


static FVECTOR	Cheild_Target_Size[] = { 
    // level 0 は無し 
    // level 1 
    { 85.f, 135.f, 85.f },	// 頭 
    { 50.f, 50.f, 50.f },	// 心臓 
    // level 2 
    { 50.f, 130.f, 50.f },	// 右腕１ 
    { 50.f, 130.f, 50.f },	// 右腕２ 
    { 50.f, 130.f, 50.f },	// 左腕１ 
    { 50.f, 130.f, 50.f },	// 左腕２ 
    { 85.f, 200.f, 85.f },	// 右足１ 
    { 70.f, 200.f, 70.f },	// 右足２ 
    { 85.f, 200.f, 85.f },	// 左足１ 
    { 70.f, 200.f, 70.f },	// 左足２ 
    // level 3 
    { 120.f, 260.f, 120.f },	// 胴体 
};
#endif
// ターゲットサイズ 
static FVECTOR SampleTrgSize[] = {
	{ 220.f, 350.f, 220.f },	// 立ち 
	{ 220.f, 350.f, 220.f },	// ダウン 
	{ 220.f, 350.f, 220.f },	// しゃがみ 
};

// 引き摺りシフト 
static FVECTOR SampleDragShift[] = {
	{ 26.483F, -393.755F, 940.002F },	// 仰向け頭16 
	{ 0.f, -113.487F, 291.057F },		// 仰向け頭 
	{ 0.f, -421.119F, 1192.692F },		// 仰向け足16 
	{ 0.f, -382.115F, 422.438F },		// 仰向け足 
	{ 0.f, -346.867F, 1192.629F },		// うつぶせ足16 
	{ 0.f, -294.03f, 463.501F },		// うつぶせ足 
	{ 45.761F, -317.266F, 946.003F },	// うつ伏せ頭16 
};

static FVECTOR 	HangShift = { 3400.f, -1465.f, 2630.f };

//-------------------------------- motion
enum {
PAR_MOT_STAND,	// 立ち
PAR_MOT_FACE_L,	// 顔振り左
PAR_MOT_FACE_R,	// 顔振り右
PAR_MOT_NECK_L,	// 首かしげ左
PAR_MOT_NECK_R,	// 首かしげ右
PAR_MOT_TALK,	// しゃべる
PAR_MOT_WING,	// 羽繕い
PAR_MOT_FLIP,	// ばたばた
PAR_MOT_MAX,	// 
};

//-------------------------------- sound
enum{	// 内蔵SE:インデックス指定用
PAR_SE_FLY11,  	// 海鳥羽ばたき１
PAR_SE_FLY12,  	// 海鳥羽ばたき２
PAR_SE_FLY13,  	// 海鳥羽ばたき３
PAR_SE_FLY14,  	// 海鳥羽ばたき４
PAR_SE_FLIP,   	// 海鳥飛び立ち羽ばたき
PAR_SE_BLEAT01, // オウム鳴き声１
PAR_SE_BLEAT02, // オウム鳴き声２
PAR_SE_BLEAT03, // オウム鳴き声３
PAR_SE_BLEAT04, // オウム鳴き声４
SE_MAX,
};

static int PAR_SE[ SE_MAX ] = { //  内蔵SE
SD_A_KMOFLY11, // 海鳥羽ばたき５
SD_A_KMOFLY12, // 海鳥羽ばたき６
SD_A_KMOFLY13, // 海鳥羽ばたき７
SD_A_KMOFLY14, // 海鳥羽ばたき８
SD_A_KMOSOR02, // 海鳥飛び立ち羽ばたき２
SD_A_KMOVOX01, // オウム鳴き声１
SD_A_KMOVOX02, // オウム鳴き声２
SD_A_KMOVOX03, // オウム鳴き声３
SD_A_KMOVOX04, // オウム鳴き声４
};




