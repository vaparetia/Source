/*
	gll.h
	ゴルルゴン

	2002/04/25 Y.Korekado
	$Id: gll.h,v 1.1.1.3 2002/11/19 11:44:11 Yoshizawa1 Exp $
*/
/*----------------------------------------------------------------*/
#include	"korekado/conv/cnctobj.h"
#include	"gllheart.h"
/*----------------------------------------------------------------*/
#define GLL_MODEL	(15808420)	/* GV_StrCode( "gll_def" ) */
#define GNO_MODEL	(2176937)	/* GV_StrCode( "gno_def" ) */
#define GME_MODEL	(9707470)	/* GV_StrCode( "gno_meca_mt" ) */
#define GLL_WEAPON	(12573349)	/* GV_StrCode( "aks_sp_gll" ) */
#define GNO_WEAPON	(980571)	/* GV_StrCode( "fms_4gno" ) */

#define BASE_MOTION	(109036)	/* GV_StrCode("gll") */
#define DAMAGE_MOTION	(0)	/* GV_StrCode("") */
#define CAPTURE_MOTION	(0)	/* GV_StrCode("") */
#define DRAG_MOTION	(0)	/* GV_StrCode("") */

#define CHILD_TARGET_NUM	(11)
#define SMP_LIFE	(128)
#define SMP_FAINT	(10)
#define SMP_FAINT_COUNT	(60*30)
#define SMP_SLEEP_COUNT	(60*60*1 + 3)
#define SMP_DOWN_DAM	(10)

//#define	SMP_TARGET_SKIP	(1)

#define GLL_TARGET_CLASS	(TARGET_LOCKON|TARGET_POWER|TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE)
//#define GLL_TARGET_CLASS	(TARGET_POWER|TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE)
#define GLL_TARGET_NUM		(10)
#define GLL_MAX_HEART		(6)
#define GLL_REGENE_TIME		(300)
#define GLL_REGENE_START_TIME		(120)

/*----------------------------------------------------------------*/
static	u_int	LightSneak = (255) | ( 255<<8 ) | ( 150<<16 ) | ( 0x80 << 24 ) ;
static	u_int	LightAlert = (255) | ( 100<<8 ) | ( 100<<16 ) | ( 0x80 << 24 ) ;
static	u_int	LightAvoid = (255) | ( 255<<8 ) | ( 100<<16 ) | ( 0x80 << 24 ) ;

#define MAX_CHAFF_LIGHT	(4)
static	u_int	LightChaff[MAX_CHAFF_LIGHT] = {
	(255) | ( 100<<8 ) | ( 255<<16 ) | ( 0x80 << 24 ) ,
	(100) | ( 255<<8 ) | ( 255<<16 ) | ( 0x80 << 24 ) ,
	(255) | ( 255<<8 ) | ( 100<<16 ) | ( 0x80 << 24 ) ,
	(100) | ( 100<<8 ) | ( 100<<16 ) | ( 0x80 << 24 ) ,
} ;

//#define SL_RANGE	(14000.0)	/* サーチライト投影距離 */
#define SL_RANGE	(1000000.0)	/* サーチライト投影距離 */
/*----------------------------------------------------------------*/
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define GLL_DEF_EYE_LENGTH		(18000)
//#define GLL_DEF_EYE_RANGE		(512)
#define GLL_DEF_EYE_RANGE		(128)

	/* 視覚情報 */
enum {
	EYE_INFO_SIGHT_OUT		=0,		/* みえない			*/
	EYE_INFO_SIGHT_OUT_HZD	=1,		/* 間に障害物が有りみえない			*/
	EYE_INFO_SIGHT_BLURR	=2,		/* ぼやけてみえる	*/
	EYE_INFO_SIGHT_IN		=3,		/* はっきりとみえる */
} ;
#define UNDER_EYE_SIGHT	512		/* 下方向視野 */
#define UPPER_EYE_SIGHT	-512			/* 上方向視野 */

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		body ;
	CONTROL		control ;
	FMATRIX		lights[2] ;

	NPCWORK		npc ;
	NAVIGATE 	navigate ;
	NAVITARGET	navitrg ;
	RADAR_CTRL	rctrl ;
	ROUTENAVI	rnavi ;

    TARGET		deftrg ;	/* 防御ターゲット */
	GM_GageSet	gage ;

    TARGET		def_child[CHILD_TARGET_NUM] ;	/* 子ターゲット*/
    POWER_TARGET	def_power[CHILD_TARGET_NUM] ;	/* 防御属性 */
    int			trg_time[GLL_TARGET_NUM] ;
    int			vanim_flag[GLL_TARGET_NUM] ;
	u_int		damaged ;

    int			id ;
    int			headmark ;

	int			think1 ;
	int			think2 ;
	int			think3 ;
	int			count3 ;

	int			tmp1 ;
	int			tmp2 ;

	int			mode ;
	int			sw_breth ;

	int			gllheart[GLL_MAX_HEART] ;

    int			eye_length ;	/* 視力 */
    int			eye_range ;	/* 視力 */
    int			facedir ;
    int			facedir_x ;
    int			sight ;	/* 視界判定 */
	int			pl_dis ;	/* プレイヤーとの距離 */
	int			pl_dir ;	/* プレイヤーへの方向 */
    int			west_dir ;
	int			find_flag ;
	int			book_dir ;
	FVECTOR		book_pos ;
	NPCADJUST	npcadjust ;
	FVECTOR		tmp_trgpos ;
	void		*searchlight ;
	FMATRIX		search_world ;
	u_int		spot_color ;
	int			sw_spot ;
	int			tmp_count ;
	int			game_count ;
	int			mot_reverse ;
	int			mot_count ;
	FMATRIX		sight_world ;

	float		speed ;
	float		z_limit ;

	TARGET		attack ;
	POWER_TARGET	power ;
	int			attack_flag ;
	int			status2 ;
	int			status ;

	CNCTOBJ		cnct ;
	int			gunhand ;

	int			end_proc ;
	int			clear_proc ;
	int			gover_proc ;
	int			head_flag ;
	int			shadow ;

} Work ;

/*----------------------------------------------------------------*/
/* stauts */
#define GLL_STATUS_GLL		0x00000001	/* ゴルルゴン */
#define GLL_STATUS_GNO		0x00000002	/* げのら */
#define GLL_STATUS_GME		0x00000004	/* メカゲノラ */
#define GLL_STATUS_STOP		0x00000010	/* 処理停止＆非表示 */

#define GLL_STATUS_FIGHT	0x10000000	/* ＶＳプレイヤーステージ */

/*----------------------------------------------------------------*/
/* stauts2 */
#define GLL_STATUS2_WEST_ADJ		0x00000001	/* 腰でアジャスト */
#define GLL_STATUS2_ALERT			0x00000002	/* プレイヤー視認 */
#define GLL_STATUS2_AVOID			0x00000004	/* プレイヤー隠れた場所探し中 */
#define GLL_STATUS2_SEARCH			0x00000008	/* プレイヤー探し中 */
#define GLL_STATUS2_SL_TRG			0x00000010	/* サーチライトターゲットに */
#define GLL_STATUS2_NO_HEAD			0x00000020	/* 頭非表示 */
#define GLL_STATUS2_NO_TRG			0x00000040	/* 無敵 */
#define GLL_STATUS2_NO_ZLIMIT		0x00000080	/* 場所リミットなし */
#define GLL_STATUS2_INVALID_CHAFF	0x00000100	/* チャフ無効 */
#define GLL_STATUS2_CHAFF_CRAZY		0x00000200	/* チャフ狂い中 */

/*----------------------------------------------------------------*/
/* damaged */
#define GLL_DAM_TRGALL		0x00000001
#define GLL_DAM_TRGONE		0x00000002
#define GLL_DAM_CHAFF		0x00000004
#define GLL_DAM_REGENE		0x00000008	/* 復元中 */
#define GLL_DAM_HEART		0x00000010	/* 心臓破壊 */
#define GLL_DAM_HIT_HEART	0x00000020	/* 心臓ヒット */
#define GLL_DAM_HIT_HEAD	0x00000040	/* メカゲノラヘッドヒット */
#define GLL_DAM_TUB			0x00000080	/* ゲノラ、たらい*/
#define GLL_DAM_ACHOO		0x00000100	/* ゲノラ、くしゃみ */

/*----------------------------------------------------------------*/
/* act->status */
// システムでは下位１６ビット使う、上位はそれぞれ好きなように
#define GLL_ACT_STATUS_TRGTIME_CLEAR	0x00010000
#define GLL_ACT_STATUS_CAHFF_DAMAGE		0x00020000
#define GLL_ACT_STATUS_HEART_DAMAGE		0x00040000

/* find_flag */
#define GLL_FIND_BUDY	0x00000001
#define GLL_FIND_BOOK	0x00000002
/*----------------------------------------------------------------*/
/* base_mar */
enum {
	GLL_MOT_STAND,	//通常立ち
	GLL_MOT_WALK,	//歩き
	GLL_MOT_HOWL,	//雄叫び
	GLL_MOT_START,	//歩き初め
	GLL_MOT_END,	//歩き終わり右左
	GLL_MOT_END_L,	//歩き終わり左右
	GLL_MOT_LOOK_L_12,	//覗き込み、左、高さ１２ｍ
	GLL_MOT_LOOK_R_12,	//覗き込み、右、高さ１２ｍ
	GLL_MOT_LOOK_L_7,	//覗き込み、左、高さ７ｍ
	GLL_MOT_LOOK_R_7,	//覗き込み、右、高さ７ｍ
	GLL_MOT_DOWN,		//共通ダウン
	GLL_MOT_DAM,		//共通ダメージ
	GLL_MOT_FIRE,		//ゴルルゴン＆メカゲノラビーム。
	GLL_MOT_DAM_CHAF,	//メカゴルチャフダメージ
	GLL_MOT_PUNCH_R,	//ゲノラ右パンチ
	GLL_MOT_PUNCH_L,	//ゲノラ左パンチ
	GLL_MOT_BOOK,		//エロ本発見
	GLL_MOT_SNEEZE,		//くしゃみ
	GLL_MOT_LOOK_F_12,	//覗き込み、正面、高さ12ｍ
	GLL_MOT_LOOK_F_16,	//覗き込み、正面、高さ16ｍ
	GLL_MOT_LOOK_F_7,	//覗き込み、正面、高さ7ｍ
	GLL_MOT_BDMH_START,	//びっくりドッキリメカ発進
	GLL_MOT_BDMH_LOOP,	//びっくりドッキリメカ発進
	GLL_MOT_BDMH_END,	//びっくりドッキリメカ発進
} ;

enum {
	SMP_TARGET_CHILD_HEAD,

	SMP_TARGET_CHILD_ARMR1,
	SMP_TARGET_CHILD_ARMR2,
	SMP_TARGET_CHILD_ARML1,
	SMP_TARGET_CHILD_ARML2,

	SMP_TARGET_CHILD_LEGR1,
	SMP_TARGET_CHILD_LEGR2,
	SMP_TARGET_CHILD_LEGL1,
	SMP_TARGET_CHILD_LEGL2,

	SMP_TARGET_CHILD_BPDY,
} ;

static int	Cheild_Target_Connect[] = { 
	HUMAN21_ATAMA,

	HUMAN21_MIGI_UDE1,
	HUMAN21_MIGI_UDE2,
	HUMAN21_HIDARI_UDE1,
	HUMAN21_HIDARI_UDE2,
	HUMAN21_MIGI_ASHI1,
	HUMAN21_MIGI_ASHI2,
	HUMAN21_HIDARI_ASHI1,
	HUMAN21_HIDARI_ASHI2,

	HUMAN21_MUNE
} ;

static FVECTOR	Cheild_Target_Shift[] = { 
	/* level 0 は無し */
	/* level 1 */
	{ 0.0F, 300.0F, 0.0F },		/* 頭 */
	/* level 2 */
	{ 0.0F, -1300.0F, 0.0F },	/* 右腕１ */
	{ 0.0F, -1300.0F, 0.0F },	/* 右腕２ */
	{ 0.0F, -1300.0F, 0.0F },	/* 左腕１ */
	{ 0.0F, -1300.0F, 0.0F },	/* 左腕２ */
	{ 100.0F, -2000.0F, -100.0F },/* 右足１ */
	{ 300.0F, -2500.0F, -200.0F },/* 右足２ */
	{ -100.0F, -2000.0F, -100.0F },/* 左足１ */
	{ -300.0F, -2500.0F, -200.0F },/* 左足２ */
	/* level 3 */
	{ 0.0F, 0.0F, 0.0F },		/* 胴体 */
} ;

static FVECTOR	GLL_Target_Shift[] = { 
	/* level 0 は無し */
	/* level 1 */
	{ 0.0F, 0.0F, 0.0F },		/* 頭 */
	/* level 2 */
	{ 0.0F, 0.0F, 0.0F },	/* 右腕１ */
	{ 0.0F, 0.0F, 0.0F },	/* 右腕２ */
	{ 0.0F, 0.0F, 0.0F },	/* 左腕１ */
	{ 0.0F, 0.0F, 0.0F },	/* 左腕２ */
	{ 0.0F, 0.0F, 0.0F },/* 右足１ */
	{ 0.0F, 0.0F, 0.0F },/* 右足２ */
	{ 0.0F, 0.0F, 0.0F },/* 左足１ */
	{ 0.0F, 0.0F, 0.0F },/* 左足２ */
	/* level 3 */
	{ 0.0F, 0.0F, 0.0F },		/* 胴体 */
} ;

static FVECTOR	Cheild_Target_Size[] = { 
	/* level 0 は無し */
	/* level 1 */
	{ 960.0F, 1300.0F, 960.0F },		/* 頭 */
	/* level 2 */
	{ 750.0F, 1500.0F, 750.0F },	/* 右腕１ */
	{ 750.0F, 1500.0F, 750.0F },	/* 右腕２ */
	{ 750.0F, 1500.0F, 750.0F },	/* 左腕１ */
	{ 750.0F, 1500.0F, 750.0F },	/* 左腕２ */
	{ 1200.0F, 2000.0F, 1200.0F },	/* 右足１ */
	{ 700.0F, 2300.0F, 1000.0F },	/* 右足２ */
	{ 1200.0F, 2000.0F, 1200.0F },	/* 左足１ */
	{ 700.0F, 2300.0F, 1000.0F },	/* 左足２ */
	/* level 3 */
	{ 2000.0F, 3000.0F, 2000.0F },	/* 胴体 */
} ;

static FVECTOR	GNO_Target_Size[] = { 
	/* level 0 は無し */
	/* level 1 */
	{ 960.0F, 1300.0F, 960.0F },		/* 頭 */
	/* level 2 */
	{ 750.0F, 1500.0F, 750.0F },	/* 右腕１ */
	{ 750.0F, 1500.0F, 750.0F },	/* 右腕２ */
	{ 750.0F, 1500.0F, 750.0F },	/* 左腕１ */
	{ 750.0F, 1500.0F, 750.0F },	/* 左腕２ */
	{ 1200.0F, 2000.0F, 1200.0F },	/* 右足１ */
	{ 1500.0F, 3300.0F, 1500.0F },	/* 右足２ */
	{ 1200.0F, 2000.0F, 1200.0F },	/* 左足１ */
	{ 1500.0F, 3300.0F, 1500.0F },	/* 左足２ */
	/* level 3 */
	{ 2000.0F, 3000.0F, 2000.0F },	/* 胴体 */
} ;

static FVECTOR	GLL_Target_Size[] = { 
	/* level 0 は無し */
	/* level 1 */
	{ 960.0F, 1300.0F, 960.0F },		/* 頭 */
	/* level 2 */
	{ 750.0F, 1200.0F, 750.0F },	/* 右腕１ */
	{ 750.0F, 1500.0F, 750.0F },	/* 右腕２ */
	{ 750.0F, 1200.0F, 750.0F },	/* 左腕１ */
	{ 750.0F, 1500.0F, 750.0F },	/* 左腕２ */
	{ 1200.0F, 2000.0F, 1200.0F },	/* 右足１ */
	{ 1000.0F, 2300.0F, 1300.0F },	/* 右足２ */
	{ 1200.0F, 2000.0F, 1200.0F },	/* 左足１ */
	{ 1000.0F, 2300.0F, 1300.0F },	/* 左足２ */
	/* level 3 */
	{ 2000.0F, 3000.0F, 2000.0F },	/* 胴体 */
} ;


/* ターゲットサイズ */
static FVECTOR SampleTrgSize[] = {
	{ 4000.0F, 10000.0F, 4000.0F },	/* 立ち */
	{ 10000.0F, 2000.0F, 10000.0F },	/* ダウン */
	{ 7000.0F, 6500.0F, 7000.0F },		/* しゃがみ */
} ;



#define	SW_BODY_NORMAL	(0x00000000)
#define	SW_BODY_DAMAGE	(0x00000001)
#define	SW_BODY_DBROKEN	(0x00000002)
#define	SW_HEART_NORMAL	(0x00000000)
#define	SW_HEART_DAMAGE	(0x00010000)
#define	SW_HEART_BROKEN	(0x00020000)

#define SW_HEART		(0xffff0000)

