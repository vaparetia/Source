/*
    prezdata.c
    大統領データ
    2001/04/23 Masafumi Okuta
    $Id: prezdata.c,v 1.1.1.3 2002/11/19 11:48:09 Yoshizawa1 Exp $
*/

enum { // モーション
PREZ_MOT_STAND = NPC_BASE_MOT_END,	// 立ち
PREZ_MOT_HOLDARM_START,			// うで組開始
PREZ_MOT_HOLDARM,			// うで組

PREZ_MOT_HOLDARM_END,			// うで組終了		5
PREZ_MOT_WALK,				// 歩く
PREZ_MOT_RUN,				// 走る
PREZ_MOT_SQUAT_START,			// しゃがみ開始
PREZ_MOT_SQUAT,				// しゃがみ

PREZ_MOT_SQUAT_END,			// しゃがみ終了		10
PREZ_MOT_LISTEN_START,			// 聞き耳開始
PREZ_MOT_LISTEN,			// 聞き耳
PREZ_MOT_LISTEN_END,			// 聞き耳終了
PREZ_MOT_FIND_START,			// 発見開始

PREZ_MOT_FIND,				// 発見			15
PREZ_MOT_PEEP_START,			// のぞき見開始
PREZ_MOT_PEEP,				// のぞき見
PREZ_MOT_PEEP_SQUAT,			// しゃがみのぞき見
PREZ_MOT_PEEP_SQUAT_END,		// しゃがみのぞき見終了

PREZ_MOT_BEHIND_F_START_L,		// 左壁張りつき開始	20
PREZ_MOT_BEHIND_F_START_R,		// 右壁張りつき開始
PREZ_MOT_BEHIND_F_POSE_L,		// 左壁張りつき
PREZ_MOT_BEHIND_F_POSE_R,		// 右壁張りつき
PREZ_MOT_BEHIND_F_END_L,		// 左壁張りつき終了

PREZ_MOT_BEHIND_F_END_R,		// 右壁張りつき終了	25
PREZ_MOT_BEHIND_B_START,		// 背中張りつき開始
PREZ_MOT_BEHIND_B_POSE,			// 背中張りつき
PREZ_MOT_BEHIND_B_END,			// 背中張りつき終了
PREZ_MOT_BEHIND_MOVE_L,			// 張りつき移動（左）

PREZ_MOT_BEHIND_MOVE_R,			// 張りつき移動（右）	30
PREZ_MOT_SIDESTEP_L,			// 横飛び（左）
PREZ_MOT_SIDESTEP_R,			// 横飛び（右）
PREZ_MOT_FOOTUP,			// 足あげ
PREZ_MOT_TUMBLE,			// 転倒		

PREZ_MOT_EVADE_U,			// 回避（上）		35
PREZ_MOT_EVADE_D,			// 回避（下）
PREZ_MOT_SLEEP1_POSE,			// ねている
PREZ_MOT_SLEEP1_END,			// ねている
PREZ_MOT_SLEEP2_POSE,			// ねている

PREZ_MOT_SLEEP2_END,			// ねている		40
PREZ_MOT_AWAKE,				// おきた

// ダメージ系
PREZ_MOT_DOWN_IDLE,			// 転倒持続
PREZ_MOT_WAKEUP,			// 起き上がり
PREZ_MOT_BOMBDMG_START,			// 爆弾ダメージ開始
PREZ_MOT_BOMBDMG_POSE,			// 爆弾ダメージ
PREZ_MOT_BOMBDMG_END,			// 爆弾ダメージ終了
PREZ_MOT_DEAD,				// 死亡
};

// 子ターゲットレベル
static int	Child_Level_Num[] = { 
	0,	// Level 0 のターゲット数 
	2,	// Level 1 のターゲット数 
	8,	// Level 2 のターゲット数 
	1	// Level 3 のターゲット数 
};

enum { // 子ターゲット部位
SMP_TARGET_CHILD_HEAD,
SMP_TARGET_CHILD_HART,
    
SMP_TARGET_CHILD_ARMR1,
SMP_TARGET_CHILD_ARMR2,
SMP_TARGET_CHILD_ARML1,
SMP_TARGET_CHILD_ARML2,

SMP_TARGET_CHILD_LEGR1,
SMP_TARGET_CHILD_LEGR2,
SMP_TARGET_CHILD_LEGL1,
SMP_TARGET_CHILD_LEGL2,
    
SMP_TARGET_CHILD_BPDY,
};

// 子ターゲット接続関節
static int Cheild_Target_Connect[] = { 
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

// 子ターゲットシフト値
static FVECTOR	Cheild_Target_Shift[] = { 
    // level 0 は無し 
    // level 1 
    { 0.0F, 30.0F, 0.0F },	// 頭 
    { 100.0F, 0.0F, 100.0F },	// 心臓 
    // level 2 
    { 0.0F, -130.0F, 0.0F },	// 右腕１ 
    { 0.0F, -130.0F, 0.0F },	// 右腕２ 
    { 0.0F, -130.0F, 0.0F },	// 左腕１ 
    { 0.0F, -130.0F, 0.0F },	// 左腕２ 
    { -10.0F, -200.0F, -10.0F },// 右足１ 
    { -20.0F, -250.0F, -20.0F },// 右足２ 
    { -10.0F, -200.0F, -10.0F },// 左足１ 
    { -20.0F, -250.0F, -20.0F },// 左足２ 
    // level 3 
    { 0.0F, 0.0F, 0.0F },	// 胴体 
};

// 子ターゲットサイズ
static FVECTOR	Cheild_Target_Size[] = { 
    // level 0 は無し 
    // level 1 
    { 85.0F, 135.0F, 85.0F },	// 頭 
    { 50.0F, 50.0F, 50.0F },	// 心臓 
    // level 2 
    { 50.0F, 130.0F, 50.0F },	// 右腕１ 
    { 50.0F, 130.0F, 50.0F },	// 右腕２ 
    { 50.0F, 130.0F, 50.0F },	// 左腕１ 
    { 50.0F, 130.0F, 50.0F },	// 左腕２ 
    { 85.0F, 200.0F, 85.0F },	// 右足１ 
    { 70.0F, 200.0F, 70.0F },	// 右足２ 
    { 85.0F, 200.0F, 85.0F },	// 左足１ 
    { 70.0F, 200.0F, 70.0F },	// 左足２ 
    // level 3 
    { 120.0F, 260.0F, 120.0F },	// 胴体 
};	

// 親ターゲットサイズ 
static FVECTOR SampleTrgSize[] = {
   { 350.0F, 800.0F, 350.0F },	// 立ち 
   { 500.0F, 200.0F, 500.0F },	// ダウン 
   { 350.0F, 380.0F, 350.0F },	// しゃがみ 
   { 100.0F, 800.0F, 100.0F },	// しゃがみ 
};

// 引き摺りシフト 
static FVECTOR SampleDragShift[] = {
    { 26.483F, -393.755F, 940.002F },	// 仰向け頭16 
    { 0.0F, -113.487F, 291.057F },	// 仰向け頭 
    { 0.0F, -421.119F, 1192.692F },	// 仰向け足16 
    { 0.0F, -382.115F, 422.438F },	// 仰向け足 
    { 0.0F, -346.867F, 1192.629F },	// うつぶせ足16 
    { 0.0F, -294.030F, 463.501F },	// うつぶせ足 
    { 45.761F, -317.266F, 946.003F },	// うつ伏せ頭16 
};

// 首締めシフト
static FVECTOR 	HangShift = { 3400.0F, -1465.0F, 2630.0F };

//-------------------------------- sound
enum{	// 内蔵SEインデックス
SE_BIKKURI,   	// びっくりマーク（！）
SE_KIAI,	// 気合い
SE_HATENA,	// 敵兵１足跡モード「ん？」gbs_a006 320
SE_ZZZ01,	// 敵兵１・いびき gbsibk01 284
SE_ZZZ02,	// 敵兵２・いびき gbsibk02 285
SE_PRESUP01,    // 大統領びっくり１「ほっ！？」
SE_PRESUP02,    // 大統領びっくり２「おぉぉ」
SE_PRESUP03,    // 大統領びっくり３「ふ！？」
SE_PRESUP04,    // 大統領びっくり４「んむぅ」
SE_PRESUP05,    // 大統領びっくり５「ほぉっ！？」
SE_PREDMG01,    // 大統領ダメージ１「うぁーっ！」
SE_PREDMG02,    // 大統領ダメージ２「ぬぅうっ！」
SE_PREDMG03,    // 大統領ダメージ３「うわっ！」
SE_PREOUT01,    // 大統領アウト「ぬぉぉぉぉ！！」
SE_MAX,
};

static int PREZ_SE[ SE_MAX ] = { // 内蔵SE
SD_E_BIKKRI01,  // びっくりマーク（！）
SD_V_PKIAI01,	// 気合い
SD_V_MAKI20,	// 敵兵１足跡モード「ん？」
SD_V_GBSIBK01,	// 敵兵１・いびき
SD_V_GBSIBK02,	// 敵兵２・いびき
SD_V_PRESUP01,  // 大統領びっくり１「ほっ！？」
SD_V_PRESUP02,  // 大統領びっくり２「おぉぉ」
SD_V_PRESUP03,  // 大統領びっくり３「ふ！？」
SD_V_PRESUP04,  // 大統領びっくり４「んむぅ」
SD_V_PRESUP05,  // 大統領びっくり５「ほぉっ！？」
SD_V_PREDMG01,  // 大統領ダメージ１「うぁーっ！」
SD_V_PREDMG02,  // 大統領ダメージ２「ぬぅうっ！」
SD_V_PREDMG03,  // 大統領ダメージ３「うわっ！」
SD_V_PREOUT01,  // 大統領アウト「ぬぉぉぉぉ！！」
};



