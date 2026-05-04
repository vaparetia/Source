/*
  ema.h
  NPCメインヘッダ

  2001/02/08 Y.Korekado
  $Id: emma.h,v 1.1.1.3 2002/11/19 11:46:03 Yoshizawa1 Exp $
*/

#ifndef __EMA_H__
#define __EMA_H__

/* NPC構造体 */
#include "../../../korekado/npc/npc.h"

/* カメラ構造体 */
#include "camera.h"

/* プレーヤー構造体 */
#include "../../../sonoyama/raiden/raiden.h"

/* アクションフラグ */
#include "emma_flag.h"

/* エマのコマンド */
#include "emma_com.h"


#include "../../include/util.h"

/* モーションリスト */
#include "emma.mh"
#include "emacap.mh"
#include "emadrag.mh"
//#include "emadam.mh"


/* デフォルトデータ名 */
#define EMA_KMS_MODEL_NAME  11752553   /* GV_StrCode( "ema_def_sh_mt" ) */
#define EMA_EVM_MODEL_NAME  5461097    /* GV_StrCode( "ema_def_mh_mt" ) */
#define EMA_BASE	    3424769    /* GV_StrCode( "emma" )    */
#define EMA_DAMG            138590     /* GV_StrCode( "emadam" ) */
#define EMA_CAPT            137569     /* GV_StrCode( "emacap" )  */
#define EMA_DRAG            4452007    /* GV_StrCode( "emadrag" ) */
#define EMA_MASUIDAN_MODEL  2512988    /* GV_StrCode( "m92_bul2" ) */
#define EMA_FACEANIME_CHARA 9984933    /* GV_StrCode( "ema_faceanime" ) */
#define EMA_FAR             107009     /* GV_StrCode( "ema" ) */

//#define BODYWORLD(a,b) ((a)->objs->objs[(b)].world)
#define BODYPOS(a,b)   ((FVECTOR*)&((a)->objs->objs[(b)].world.m[W]))
#define MAR(_a,_b) (((_a)&0xffffff00)|((_b)&0x000000ff))

/* フラグ関係 */
#define EMA_SetFlag(_f)   (work->flag |=    (_f))
#define EMA_ResetFlag(_f) (work->flag &=   ~(_f))
#define EMA_Flag(_f)      (work->flag &     (_f))
#define EMA_PFlag(_f)     (work->flag_prv & (_f))
#define EMA_ResetSetFlag(_reset,_set)   (EMA_ResetFlag(_reset),EMA_SetFlag(_set))

#define EMA_BLAST 
#define	EMA_WP_BLAST	(WP_C4BOMB|WP_NIKITA|WP_STINGER|WP_RGB6|WP_GRENADE)

/* 角度関係 */
#define DEG2RAD(_s) (float)((_s)*(float)M_PI/180.0f)
#define RAD2ANG(_s) (short)((_s)*2048.0f/(float)M_PI)
#define ANG2RAD(_s) (float)((_s)*(float)M_PI/2048.0f)



#define ARMS_MASK ((1<<HUMAN21_MIGI_KATA  )|(1<<HUMAN21_MIGI_UDE1  )| \
		   (1<<HUMAN21_MIGI_UDE2  )|(1<<HUMAN21_MIGI_TE    )| \
		   (1<<HUMAN21_HIDARI_KATA)|(1<<HUMAN21_HIDARI_UDE1)| \
		   (1<<HUMAN21_HIDARI_UDE2)|(1<<HUMAN21_HIDARI_TE  ))
#define BODY_MASK ((1<<HUMAN21_ONAKA)|(1<<HUMAN21_MUNE)|(1<<HUMAN21_KUBI)|(1<<HUMAN21_ATAMA))


/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

enum
{
    EMA_TARGET_SIZE_STAND,
    EMA_TARGET_SIZE_DOWN ,
    EMA_TARGET_SIZE_SQUAT,
} ;

/* ターゲット */
enum {
    EMA_TARGET_LVL0,

    EMA_TARGET_LVL1=0,
    EMA_TARGET_CHILD_HEAD = EMA_TARGET_LVL1,
    EMA_TARGET_CHILD_HART,

    EMA_TARGET_LVL2,
    EMA_TARGET_CHILD_ARMR1=EMA_TARGET_LVL2,
    EMA_TARGET_CHILD_ARMR2,
    EMA_TARGET_CHILD_ARML1,
    EMA_TARGET_CHILD_ARML2,
    EMA_TARGET_CHILD_LEGR1,
    EMA_TARGET_CHILD_LEGR2,
    EMA_TARGET_CHILD_LEGL1,
    EMA_TARGET_CHILD_LEGL2,

    EMA_TARGET_LVL3,
    EMA_TARGET_CHILD_BPDY=EMA_TARGET_LVL3,

    EMA_TARGET_CHILD_NUM
} ;

#define EMA_MAX_HIDE  16

#define EMA_MAX_VITALITY 100
#define EMA_VITAL_UP     2
#define EMA_VITALITY     EMA_MAX_VITALITY
#define EMA_FAINT	(10)
#define EMA_FAINT_COUNT	( 30*60*5/TIME_BASE)
#define EMA_SLEEP_COUNT	(120*60*5/TIME_BASE)
#define EMA_DOWN_DAM     8

#define EMA_STARE_CNT    (2*60*5/TIME_BASE)
#define EMA_NONSTARE_CNT (1*60*5/TIME_BASE)

#define EMA_ATTACK_RAD   650.0f

#define EMA_MAX_ENEMY    20

#define EMA_INVINCIBLE_TIME (2*60*5/TIME_BASE)

enum damg_amount_t
{
    EMA_PUNCH_DAM = 1,
    EMA_KICK_DAM  = 3,
    EMA_THROW_DAM = 5,
} ;

/* メッセージ */
enum message_t
{
    EMA_M_REGIST_POSTION,     /* eeply.c と通信するための情報 エマの位置 */
    EMA_M_REGIST_MOTION_NUM,  /* eeply.c と通信するための情報 エマのモーション番号 */
    EMA_M_REGIST_TRIGGER,     /* eeply.c と通信するための情報 トリガー */
    EMA_M_SET_TRIGGER,
    EMA_M_INVISBLE_INACT,     /*  */
    EMA_M_VISBLE_ACT,
    EMA_M_INVINCIBLE_ON,      /* 無敵ON  */
    EMA_M_INVINCIBLE_OFF,     /* 無敵OFF */
    EMA_M_GIVE_DAMAGE,        /* ダメージ数を引数としてダメージを受ける */

    EMA_M_FORCE_MOVE,         /* 強制瞬間移動 */

    EMA_M_ENABLE_RECOVER,     /* 10 回復可能になる   */
    EMA_M_DISABLE_RECOVER,    /*    回復不可能になる */

    EMA_M_FORCE_UNLINK,       /*    強制手繋ぎを止める */

    EMA_M_ENABLE_STUNGRENADE ,/*    スタン有効 */
    EMA_M_DISABLE_STUNGRENADE,/*    スタン無効 */

    EMA_M_FORCE_WAKEUP,       /* 15 強制的に起こす */
    EMA_M_FORCE_LINK,         /*    強制的に手を繋ぐ (毎フレーム可能) */
    EMA_M_FORCE_HIDE,         /*    強制的に起こす */
    EMA_M_SET_LIFE,           /*    ライフをセットする */
    EMA_M_FORCE_DIR,          /*    強制的に向いている方向を指定する */

    EMA_M_VIBRATE_ON,         /* 20 振動オン */
    EMA_M_VIBRATE_OFF,        /*    振動オフ */

    EMA_M_EXEC_PROC,           /*   プロックを実行する */

    EMA_M_INVISBLE_SNIPE_INACT,/*    狙撃用アクト停止メッセージ */
    EMA_M_VISBLE_SNIPE_ACT,    /*    狙撃用アクト開始メッセージ */

    EMA_M_DISABLE_LINK ,       /* 25 手繋ぎ用 手繋ぎ不可能状態にする */
    EMA_M_ENABLE_LINK  ,       /*    手繋ぎ用 手繋ぎ可能状態にする   */

    EMA_M_RELEASE_C4  ,        /*    背中設置 C4解除   */

    EMA_M_DISABLE_WANDER,      /*    徘徊不可能メッセージ */
    EMA_M_ENABLE_WANDER,       /*    徘徊可能メッセージ   */
} ;

/* プロック */
enum proc_t
{
    EMA_P_GAMEOVER, /* ゲームオーバー時のプロック */

    EMA_P_SNIPER  , /* 狙撃用プロック */
    EMA_P_SNIPER_CLAYMORE = EMA_P_SNIPER, 
    EMA_P_SNIPER_ENEMY1,                 /*敵起動詰め所Ａ                 */
    EMA_P_SNIPER_CYPHER1,		 /*ガンサイファー起動詰め所Ａ     */
    EMA_P_SNIPER_CYPHER2,		 /*ガンサイファー起動詰め所Ａ裏側 */
    EMA_P_SNIPER_CYPHER3,		 /*ガンサイファー起動浮き橋２     */
    EMA_P_SNIPER_ENEMY2,		 /*敵起動詰め所Ｂ                 */
    EMA_P_SNIPER_CYPHER4,		 /*ガンサイファー起動詰め所Ｂ     */
    EMA_P_SNIPER_CYPHER5,		 /*ガンサイファー起動詰め所Ｂ裏側 */
    EMA_P_SNIPER_CYPHER6,		 /*ガンサイファー起動浮き橋３     */
    EMA_P_SNIPER_VAMP,			 /*ヴァンプ登場                   */
    EMA_P_SNIPER_BRIDGE1,		 /*浮き橋１ＯＦＦ                 */
    EMA_P_SNIPER_BRIDGE2,		 /*浮き橋２ＯＮ                   */
    EMA_P_SNIPER_BRIDGE3,		 /*浮き橋２ＯＦＦ                 */
    EMA_P_SNIPER_BRIDGE4,		 /*浮き橋３ＯＮ                   */
    EMA_P_SNIPER_DISCOVER1,		 /*浮き橋１                       */
    EMA_P_SNIPER_DISCOVER2,		 /*浮き橋２                       */

    EMA_P_SNIPER_ATTACK,		 /*敵兵サイファーが攻撃している   */
    EMA_P_SNIPER_NORMAL,		 /*敵兵サイファーが攻撃していない */

    EMA_P_N_PROC
} ;

/* 音声 */
enum voice_t
{
    EMA_VC221,/*手繋 寝てる２       「……うーん…………お兄ちゃん……」*/
    EMA_VC231,/*手繋 寝てる３       「あぁ！金星蟹が……！」*/
    EMA_VC251,/*手繋 殴られた１     何するの」*/
    EMA_VC261,/*手繋 殴られた２     痛いじゃない！」*/
    EMA_VC281,/*手繋 殴られた４     サイテー」*/

    EMA_VC291,/*手繋 LIFE少ない１   「もう……ダメ……」*/
    EMA_VC301,/*手繋 LIFE少ない２   「うぅぅぅ……」*/
    EMA_VC351,/*手繋 座り込み３      ……少し……休ませて……」*/
    EMA_VC361,/*手繋 座り込み４      ……つかれた……」*/
    EMA_VC411,/*手繋 いたずら１      ……ヘンタイ！」*/

    EMA_VC431,/*手繋 いたずら３      触らないで！」*/
    EMA_VC451,/*手繋 虫１            「虫！虫キライ」*/
    EMA_VC461,/*手繋 虫２            「虫ダメ！虫イヤ！虫キライ」*/
    EMA_VC471,/*手繋 虫３            「虫キライ私、絶対動かないから」*/
    EMA_VC571,/*狙撃 LIFE少ない１    「あぁ……もうダメ……」*/

    EMA_VC581,/*狙撃 LIFE少ない２    「なんとかして！」*/
    EMA_VC601,/*狙撃 LIFE少ない４    「ふぅ……ふぅ……」*/
    EMA_VC611,/*狙撃 LIFE回復１      「ごめん……少し休ませて……」*/
    EMA_VC621,/*狙撃 LIFE回復２      「もう少しだけ……休ませて……」*/
    EMA_VC631,/*狙撃 LIFE回復３      「もう少し……お願い……」*/

    EMA_VC651,/*狙撃 ライデンから攻撃１   ちょっと！殺す気」*/
    EMA_VC671,/*狙撃 ライデンから攻撃３   何なのよ、あなたサイテー」*/
    EMA_VC681,/*狙撃 ライデンから攻撃４   もうイヤ」*/
    EMA_VC711,/*狙撃 ヴァンプ１        「うぅぅぅ……」*/
    EMA_VC721,/*狙撃 ヴァンプ２        「苦しい……」*/

    EMA_VC731,/*狙撃 ヴァンプ３        「……助けて……」*/

    EMA_V_N_VOICE
} ;


/* スネークのステータス  GetNPCSnakeStatus()の返り値  */
enum {
	NPCSNAKE_SCNSTATUS_NORMAL=0,

	// 逃走中
	NPCSNAKE_SCNSTATUS_RUNAWAY,

	// 寝ています
	NPCSNAKE_SCNSTATUS_SLEEP,

	// ダメージ中
	NPCSNAKE_SCNSTATUS_DAMAGE,

	// 気絶中
	NPCSNAKE_SCNSTATUS_FAINT,

	// ライデンに反撃
	NPCSNAKE_SCNSTATUS_COUNTERATTACK,

	// 銃を構えている
	NPCSNAKE_SCNSTATUS_POINT_TARGET,

	// 銃を撃っている
	NPCSNAKE_SCNSTATUS_SHOOT,

	// ライデンの攻撃によるダメージ中
	NPCSNAKE_SCNSTATUS_DAMAGE_BY_RAIDEN,
};



enum voice_flag_t
{
    EMA_VCF_SNIPER_EMMA_NORM = 0,
    EMA_VCF_SNIPER_EMMA_WISP ,
    EMA_VCF_SNIPER_SNAKE_WISP,
} ;



enum stage_t
{
    EMA_STG_W28A,
    EMA_STG_W25D,
    EMA_STG_W31B,
    EMA_STG_W31D,
    EMA_STG_W31F,
} ;





typedef struct
{
    GV_ACT_EX           actor     ;
    OBJECT              body      ;
    CONTROL             control   ;
    FMATRIX             lights[2] ;
    FVECTOR             findpos   ;     /* 敵兵が見つけるための点 立1001/座750/寝200 */
    float               height    ;     /* 腰の高さを調整するためのもの */
    int                 interp    ;     /* 補間フレーム数     */
    int                 ground    ;     /* 何フレームの間に宙に浮いているか */

    NPCWORK             npc       ;
    NAVIGATE            navigate  ;
    NAVITARGET          navitrg   ;
    HOMING_TRG          homing    ;     /* ホーミングターゲット(SKIPしているコントロール参照用) */
    RADAR_CTRL          rctrl     ;
    GM_GageSet          gage      ;     /* ライフゲージ */
    int                 headmark  ;

    HZX_PAT            *r_patrol  ;     /* ルート情報(ヘッダ情報) */
    HZX_PTP            *r_point   ;     /* ルート情報(目標)       */
    HZX_PTP            *route     ;     /* 通過ルート情報(通過)   */

    TARGET              deftrg  ;       /* 防御ターゲット     */
    CAPTURE_TARGET      capture ;       /* 捕まりターゲット   */
    TARGET              pushtrg ;       /* ぶつかりターゲット */
    TARGET              def_child[EMA_TARGET_CHILD_NUM] ; /* 子ターゲット*/
    TARGET             *offtrg ;        /* 当たった時のターゲット */

    short               time     ;      /* 思考ごとの経過時間 */
    short               tic      ;
    int                 pl_dis   ;      /* プレイヤーとの距離 */
    int                 pl_dir   ;      /* プレイヤーへの方向 */
    short               pl_turn  ;      /* プレーヤー歩く方向変化(手繋ぎイベント専用) */ 

    u_int               flag        ;   /* 状態フラグ */
    u_int               flag_prv    ;   /* 前の状態フラグ */

    short               vital_max   ;   /* 命の最大値         */
    short               damage_gun  ;   /* ダメージ値(銃系  ) */
    short               damage_bomb ;   /* ダメージ値(ボム系) */
    short               non_damage  ;   /* 無敵時間           */

    int                *voice_le  ;     /* 音声ストリームデータ */
    int                 voice[ EMA_V_N_VOICE ]; // Endian swapped voice_le
    short               voice_id  ;     /* 音声SE   ID                    */
    short               voice_tim ;     /* 音声SE   発生タイミング        */
    short               voice_str ;     /* 音声 ストリームのハンドラ      */
    short               voice_cnt ;     /* 音声 ストリームのカウンタ(汎用)*/
    short               voice_flg ;     /* 音声 フラグ (enum voice_flg_t) */
    int                 voice_code ;    /* 音声 ストリームID */
    int                 voice_proc ;    /* 音声終了プロック */
    int                 voice_chk  ;    /* 音声鳴らしていらない音声のビットが立ってる */

    int                 proc[EMA_P_N_PROC] ;
    int                 proc_time ;

    void               *arm_ik     ; /* IK用ワークメモリ */
    FVECTOR             pos_adjust ; /* 位置補正 */
    FVECTOR             vel_adjust ; /* 速度補正 */
    FVECTOR             head_dir   ; /* 顔の向く方向(頭,首クォータニオン)*/
    FVECTOR             breast_dir ; /* 顔の向く方向(胸,腹クォータニオン)*/
    FVECTOR             eye_pos    ; /* 目を向ける位置  */

    void               *face_h     ; /* 顔アニメワークメモリ */
    void               *hair_obj   ; /* 映り込み用モデル ワークメモリ */
    void               *glass_obj  ; /* 映り込み用モデル ワークメモリ */

    FVECTOR             prev_pos   ; /* 歩く量の変化 */
    int                 diff_turn  ; /* 歩く方向変化(手繋ぎイベント専用) */ 
    short               frce_turn  ; /* 歩く方向変化(手繋ぎイベント専用) */ 
    short               prev_turn  ; /* 歩く方向変化(手繋ぎイベント専用) */ 
    float               walk_speed ; /* 歩くスピード(狙撃イベント専用)   */ 
    short               pose       ; /* 体勢(0:ねる1:座り2:立ち) */
    short               walk_cnt   ; /* 強制的に歩く フレーム数                  */
    short               stay_cnt   ; /* 強制的に止まる フレーム数（狙撃イベント）*/
    short               stare_cnt  ; /* プレーヤーが自分を見るフレーム数 */
    short               fake_cnt   ; /* フェイしないフレーム（狙撃イベント）*/
    short               blink_tic  ; /* 次の目ぱちのtic emma_dps.cで使用  */
    short               stop_time  ; /* 狙撃の詰所で止まっている時間(ストリーム回避) */
    int                 shadow_flg ;

    float               wall_dist  ; /* 前回の壁までの距離 壁止まりで必要 */
    float               wall_rot   ; /* 壁向き限度の設定   壁止まりで必要 */

    int                 vib_time   ; /* 鼓動の間隔を早めるため */
    int                 vib_rate   ;

    int                 mar_mtn    ; /* 前のフレームのモーション番号 & MAR*/

    /* 足跡関係 */
    int                 foot_flag  ; /* 足跡を出す */
    int                 foot_prev  ; /* 足の状態   */

    /* スプレー */
    short               cold_stare ; /* 背けるフレーム */
    short               cold_count ; /* スプレーを掛けられている */

#if 0   /* ホロ本反応がなくなったので */
    /* エロ本反応 */
    GM_BOMB            *book       ; /* エロ本の位置などの情報 */
    FVECTOR             book_pos   ;
#endif

    FVECTOR             hide[EMA_MAX_HIDE]   ; /* 隠れ場所（手繋ぎイベント用）位置     */
    int                 hide_g[EMA_MAX_HIDE] ; /* 隠れ場所（手繋ぎイベント用）グループ */
    int                 hide_a[EMA_MAX_HIDE] ; /* 隠れ場所（手繋ぎイベント用）アドレス */
    int                 n_hide ;               /* 隠れ場所の登録数（手繋ぎイベント用） */

    int                 stage ;                /* ステージ番号 */


    /* プレーヤー制御情報 */
    short               ply_stare_cnt  ; /* 強制的にエマを見る  */
    short               ply_stare_rotx ; /* エマを見る方向      */
    short               ply_stare_roty ; /* エマを見る方向      */
    FVECTOR             ply_prv_pos    ; /* プレーヤーの前フレームの位置 */


    /* 敵兵（狙撃イベント） */
    int                 enemy[EMA_MAX_ENEMY] ;
    int                 enemy_flg ;

    /* プレーヤ参照用 */
    FVECTOR *ext_pos ; /* エマの位置を教えられるように */
    int     *ext_trg ; /* 現在のトリガー */
    int      prv_trg ; /* 前のトリガー トリガーの変化を検知するため */
    int     *ext_mtn ; /* モーション番号 */
} Work ;


/*inline functions*/
static inline int EMA_RecalcDir( int dir )
{
    dir &= 4095 ;
    dir -= dir > 2048 ? 4096 : 0 ;
    return dir ;
}

static inline int EMA_DirectionDiff( int dir )
{
    dir = EMA_RecalcDir( dir ) ; 
    dir += dir>2048 ? -4096 : dir<-2048 ? 4096 : 0 ;
    return dir ;
}


/* Proto Types */

/* emma_ini.c */
extern FMATRIX *EMA_GetConnectObjMatrix( Work *work, TARGET *def ) ;
extern int  EMA_InitNPC( Work *work, int name, int where ) ;
extern int  EMA_InitPosition( Work *work, int name, int where ) ;
extern int  EMA_InitControl( Work *work, int name, int where ) ;
extern int  EMA_InitTarget( Work *work, int name, int where ) ;
extern int  EMA_InitFlags( Work *work, int name, int where ) ;
extern int  EMA_InitParams( Work *work, int name, int where ) ;
extern int  EMA_InitInfoDisp( Work *work, int name, int where ) ;
extern int  EMA_InitFaceAnime( Work *work, int name, int where ) ;
extern int  EMA_InitRoute( Work *work, int name, int where ) ;
extern int  EMA_InitEquipment( Work *work, int name, int where ) ;
extern int  EMA_InitHoming( Work *work, int name, int where ) ;
extern int  EMA_EffectStart( Work *work, int where ) ;
extern int  EMA_InitVoice( Work *work, int name, int where ) ;


/* emma_thk.c */
extern void EMA_Think( Work *work ) ;


/* emma_act.c */
extern void EMA_ActLayerMotion( Work *work, int mot, int start, int flag ) ;
extern void EMA_ActControl( Work *work ) ;
extern void EMA_Action( Work *work ) ;
extern int  EMA_ActCheckDamage( NPCWORK *npc ) ;
extern int  EMA_ActCheckPad( NPCWORK *npc ) ;
extern void EMA_ActDamage( NPCWORK *npc, int time ) ;
extern void EMA_SetActMotion( NPCWORK *npc, int mar, int mot_num ) ;
extern void EMA_ActLoopMotion( NPCWORK *npc, int time ) ;
extern void EMA_ActOneTimeMotion( NPCWORK *npc, int time ) ;


/* emma_msg.c */
extern void EMA_Message( Work *work ) ;
extern void EMA_SendMessageFaceAnimePlayMotion( Work *work, int motion ) ;
extern void EMA_SendMessageFaceAnimeSightControl( Work *work, int type,
						  int time, FVECTOR *pos ) ;
extern void EMA_SendMessageExecProc( Work *work, int proc ) ;

extern void EMA_SendMessageWetHairInvisible( Work *work, int buffer ) ;
extern void EMA_SendMessageDryHairInvisible( Work *work, int buffer ) ;
extern void EMA_SendMessageInvisbleWetHair(  Work *work ) ;

/* emma_rcg.c */
extern void EMA_Recognize( Work *work ) ;


/* emma_dsp.c */
extern void EMA_Display( Work *work ) ;

/* emma_clb.c */
extern void EMA_TargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;


/* emma_utl.c */
extern void  EMA_SetAdjustPosition( Work *work, FVECTOR *aim, int flames ) ;
extern int   EMA_GameOverCheck( Work *work, int pad ) ;
extern void  EMA_SetInvincible( Work *work ) ;
extern int   EMA_DamageGameOver( Work *work, int pad ) ;
extern int   EMA_DamageVitality( Work *work, int damage, int pad ) ;
extern void  EMA_RecoverVitality( Work *work, int life ) ;
extern int   EMA_SetNaviTarget( Work *work, int addr, FVECTOR *pos ) ;
extern void  EMA_SetRoutePatrol( Work *work, int route_id ) ;
extern void  EMA_SetRoutePoint( Work *work, int point_id ) ;
extern int   EMA_GetDirFromRoute( Work *work, float length ) ;
extern float EMA_CheckDestFloor( Work *work, FVECTOR *dest ) ;
extern int   EMA_ThinkCheckEnemy( Work *work ) ;
extern void  EMA_Pushed( Work *work, int se ) ;
extern int   EMA_GetNearestHide( Work *work, float *min ) ;
extern int   EMA_CheckScreen( Work *work ) ;
extern int   EMA_CheckControl( Work *work, CONTROL *c, float dist, FVECTOR *v ) ;
extern CONTROL *EMA_NearControl( Work *work, float dist, FVECTOR *v ) ;
extern CONTROL *EMA_NearEneControl( Work *work, float dist, FVECTOR *v ) ;
extern int  EMA_Navi( Work *work ) ;
extern void EMA_UtilStopStream( Work *work ) ;
extern int  EMA_UtilStartStreamIdx( Work *work, int idx ) ;
extern int  EMA_UtilStartStream( Work *work, int id ) ;

extern void EMA_ThinkDestinatePos( Work *work,
				   FVECTOR *diff, FVECTOR *aim, FMATRIX *world,
				   int onstep ) ;

extern void EMA_UtilFullReset( Work *work ) ;
extern void EMA_UtilForceMove( Work *work, float x, float y, float z ) ;
extern void EMA_UtilBullet( Work *work, FVECTOR *aim ) ;
extern void EMA_ThinkResetDiff( Work *work, int move ) ;
extern int  EMA_UtilDestinateToWall( Work *work, FVECTOR *dir ) ;


/* emma_dbg.c */
extern int EMA_DbgInvincible ;
extern void EMA_InitDbgConfiguration() ;
extern void EMA_DbgDispStatus( Work *work ) ;
extern void EMA_DbgDispEvent( Work *work ) ;


/* emma_slp.c */
extern void  EMA_Sleep( Work *work ) ;



/* command/emma_com.c */
extern void EMA_InitCommandWork( void *ptr ) ;
extern void EMA_FreeCommandWork() ;


/* effect/emma_anm.c */
extern void  EMA_ActFaceAnimation( void *handler ) ;
extern void *EMA_InitFaceAnimation( DG_EVMOBJ *evmobj,
				    int fanim_id,
				    int mouth_id ) ;
extern void EMA_FreeFaceAnimation( void *work ) ;
extern void EMA_ActFaceCommand( Work *work, int message[] ) ;

/* effect/emma_ik.c */
extern int   EMA_ActPuppetIK( void *work, int flag ) ;
extern void  EMA_FreePuppetIK( void *ptr ) ;
extern void *EMA_MakePuppetIK( CONTROL *ctrl, OBJECT *body ) ;
extern void  EMA_ResetPuppetIK( void *ptr ) ;

/* effect/emma_mir.c */
extern void *EMA_InitMirrorObj( DG_OBJS *objs, int model, int num ) ;
extern void  EMA_FreeMirrorObj( void *handler )  ;
extern void  EMA_ActMirrorObj( void *handler ) ;


/* sonoyama/plugin/eeply.c */
extern void PLY_EE_ResetGraspModeAndTrigger() ;

/* system/libhzx/navigate.c */
extern void HZX_Pos2Zone( FVECTOR *pos, int *grp, int *zon ) ;

/* source/user/kano/ik/ik_sub.c */
extern void Leg_IKcalc(CONTROL *control,OBJECT *obj) ;
extern void Leg_IKcalcForEmma(CONTROL *control,OBJECT *obj) ;

/* okajima/effect/ */
extern void *NewFootPrint( OBJECT *body, CONTROL *control,
			   int *flag, int time ) ;

/* skoba/bullet/bullet.c */
extern void *NewBullet( FMATRIX *, u_int, u_int, u_int,
			u_int, u_int, u_int, int ) ;
extern void *NewShadow( DG_OBJ *lfoot, DG_OBJ *rfoot,
			CONTROL *c, FMATRIX *lgt, int *flag ) ;


/* kano/npc_snake/npc_snake.c */
extern int GetNPCSnakeStatus( void ) ;

/* korekado/npc/npcact.c */
extern void NPC_SetSlopeRotX( CONTROL *ctrl ) ;
extern void NPC_ReSetSlopeRotX( CONTROL *ctrl ) ;



/*Reference Global Variable*/
#ifdef __MAIN_FILE__


FVECTOR EMA_Shifts[] = {
    { -430.0f, 0.0f, -600.0f, 1.0f },	/* 静止シフト */
    { -400.0f, 0.0f, -200.0f, 1.0f },	/* 左周り用シフト */
    { -500.0f, 0.0f, -800.0f, 1.0f },	/* 走りシフト */
    { -630.0f, 0.0f,  500.0f, 1.0f },	/* 回り込みシフト */
    {  500.0f, 0.0f, -600.0f, 1.0f },	/* 回り込みシフト */
    { -370.0f, 0.0f, -500.0f, 1.0f },	/* 階段シフト */
    { -430.0f, 0.0f, -550.0f, 1.0f },	/* 壁あたり静止シフト */
    //{ -370.0f, 0.0f, -100.0f, 1.0f },	/* 階段静止シフト */
} ;
FVECTOR EMA_HandOffset = { 50.0f, -80.0f, -80.0f, 1.0f } ;


#else   /* __MAIN_FILE__ */


extern FVECTOR EMA_Shifts[] ;
extern FVECTOR EMA_HandOffset ;

#endif  /* __MAIN_FILE__ */


//#define printf(args...)


#endif  /* __EMA_H__ */
