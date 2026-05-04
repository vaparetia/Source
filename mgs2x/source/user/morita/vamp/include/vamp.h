/*
  vamp.h
  狙撃イベントヴァンプ メインヘッダ

  2001/03/23 T.Morita
  $Id: vamp.h,v 1.1.1.3 2002/11/19 11:46:36 Yoshizawa1 Exp $
*/

#ifndef __VMPS_H__
#define __VMPS_H__

#include "../../include/util.h"

/* NPC構造体 */
#include "../../../korekado/npc/npc.h"

/* プレーヤー構造体 */
#include "../../../sonoyama/raiden/raiden.h"

/* エマコマンド関数 */
#include "../../emma/include/emma_com.h"

/* アクションフラグ */
#include "vamp_flag.h"

/* モーションリスト */
#include "w32a_vamp.mh"

/* デフォルトデータ名 */
#define VMPS_KMS_MODEL_NAME  12663374   /* GV_StrCode( "rai_def" ) */
#define VMPS_EVM_MODEL_NAME  13285730   /* GV_StrCode( "vmp_coat_mh_mt" ) */
#define VMPS_BASE_MOTION	    10827608   /* GV_StrCode( "w32a_vamp" )    */
#define VMPS_MASUIDAN_MODEL  2512988    /* GV_StrCode( "m92_bul2" ) */
#define VMPS_FACEANIME_CHARA 10054625   /* GV_StrCode( "vmp_faceanime" ) */
#define VMPS_FAR_MOTION      107009     /* GV_StrCode( "ema" ) */

//#define BODYWORLD(a,b) ((a)->objs->objs[(b)].world)
#define BODYPOS(a,b)   ((FVECTOR*)&((a)->objs->objs[(b)].world.m[W]))
#define DEGtoRAD(_a) ((_a)*(float)M_PI/360.0f)

/* フラグ関係 */
#define VMPS_SetFlag(_f)   (work->flag |=  (_f))
#define VMPS_ResetFlag(_f) (work->flag &= ~(_f))
#define VMPS_Flag(_f)      (work->flag &   (_f))
#define VMPS_ResetSetFlag(_reset,_set)   (VMPS_ResetFlag(_reset),VMPS_SetFlag(_set))



/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

/* ターゲット */
enum {
    VMPS_TARGET_LVL0,

    VMPS_TARGET_LVL1=0,
    VMPS_TARGET_CHILD_HEAD = VMPS_TARGET_LVL1,
    VMPS_TARGET_CHILD_HART,

    VMPS_TARGET_LVL2,
    VMPS_TARGET_CHILD_ARMR1=VMPS_TARGET_LVL2,
    VMPS_TARGET_CHILD_ARMR2,
    VMPS_TARGET_CHILD_ARML1,
    VMPS_TARGET_CHILD_ARML2,
    VMPS_TARGET_CHILD_LEGR1,
    VMPS_TARGET_CHILD_LEGR2,
    VMPS_TARGET_CHILD_LEGL1,
    VMPS_TARGET_CHILD_LEGL2,

    VMPS_TARGET_LVL3,
    VMPS_TARGET_CHILD_BPDY=VMPS_TARGET_LVL3,

    VMPS_TARGET_CHILD_NUM
} ;

#define VMPS_MAX_HIDE  4

#define VMPS_MAX_VITALITY 100
#define VMPS_VITAL_UP     2
#define VMPS_VITALITY     VMPS_MAX_VITALITY
#define VMPS_FAINT	(10)
#define VMPS_FAINT_COUNT	(60*10*5/TIME_BASE)
#define VMPS_SLEEP_COUNT	(60*10*5/TIME_BASE)
#define VMPS_DOWN_DAM     8

#define VMPS_STARE_CNT    (2*60*TIME_BASE/5)
#define VMPS_NONSTARE_CNT (1*60*TIME_BASE/5)

#define VMPS_MAX_ENEMY    4

enum damg_amount_t
{
    VMPS_PUNCH_DAM = 1,
    VMPS_KICK_DAM  = 3,
    VMPS_THROW_DAM = 5,
} ;

/* メッセージ */
enum message_t
{
    VMPS_M_INVISBLE_INACT,    /* 非表示にし 行動を止める */
    VMPS_M_VISBLE_ACT,        /* 表示にし 行動を再開     */
    VMPS_M_CAPTURE_EMMA,      /* エマを強制的に捕まえる  */
} ;

/* プロック */
enum proc_t
{
    VMPS_P_GAMEEND, /* ゲーム終了時のプロック（ヴァンプをやっつけた） */

    VMPS_P_N_PROC
} ;


typedef struct
{
    GV_ACT_EX           actor     ;
    OBJECT              body      ;
    CONTROL             control   ;
    FMATRIX             lights[2] ;

    NPCWORK             npc       ;
    NAVIGATE            navigate  ;
    NAVITARGET          navitrg   ;
    RADAR_CTRL          rctrl     ;
    HOMING_TRG          homing    ;  /* ホーミング部位ターゲット */
    
    GM_GageSet        	gage      ;     /* ライフゲージ */
    int			headmark  ;

    TARGET              deftrg  ;       /* 防御ターゲット */
    CAPTURE_TARGET      capture ;       /* 捕まりターゲット */
    TARGET              pushtrg ;       /* ぶつかりターゲット */
    TARGET              def_child[VMPS_TARGET_CHILD_NUM] ;       /* 子ターゲット*/

    short               time     ;      /* 思考ごとの経過時間(0.1sec単位)*/
    short               tic      ;      /* 思考ごとの経過時間(0.1sec以下)*/
    u_int               flag     ;      /* 状態フラグ  */
    short               voice_id  ;     /* 音声ID */
    short               voice_tim ;     /* 音声 発生タイミング */
    short               vital_max ;     /* 命の最大値  */
    short               vital_m9  ;     /* M9の最大値  */
    short               damage      ;     /* ダメージ    */
    short               damage_body ;     /* 体ダメージ  */
    short               damage_m9      ;  /* M9ダメージ  */
    short               damage_m9_body ;  /* M9体ダメージ  */

    short               hang_tim  ;     /* 首締め-時間 */

    int                 proc[VMPS_P_N_PROC] ;
    FVECTOR             pos_adjust ;  /* 位置補正 */
    FVECTOR             head_dir   ;  /* 顔を向く方向（頭,首 の クォータニオン） */
    FVECTOR             breast_dir ;  /* 胸を向く方向（胸,腹 の クォータニオン） */
    short               aim_pose   ;  /* ポーズ番号(姿勢) */
    short               blink_tic  ;  /* 次の目ぱちのtic vamp_dps.cで使用 */

    HZX_D_FLOOR        *d_floor ;    /* エマのための上下床 */
} Work ;


/*inline functions*/
static inline int VMPS_RecalcDir( int dir )
{
    dir &= 4095 ;
    dir -= dir > 2048 ? 4096 : 0 ;
    return dir ;
}

/* Proto Types */

/* vamp_ini.c */
extern int  VMPS_InitNPC( Work *work, int name, int where ) ;
extern int  VMPS_InitPosition( Work *work, int name, int where ) ;
extern int  VMPS_InitControl( Work *work, int name, int where ) ;
extern int  VMPS_InitTarget( Work *work, int name, int where ) ;
extern int  VMPS_InitParams( Work *work, int name, int where ) ;
extern int  VMPS_InitInfoDisp( Work *work, int name, int where ) ;
extern int  VMPS_InitFaceAnime( Work *work, int name, int where ) ;
extern int  VMPS_InitDynamicFloor( Work *work, int name, int where ) ;
extern int  VMPS_InitHomingTarget( Work *work, int name, int where ) ;

/* vamp_thk.c */
extern void VMPS_Think( Work *work ) ;

/* vamp_act.c */
extern void VMPS_ActControl( Work *work ) ;
extern void VMPS_Action( Work *work ) ;
extern int  VMPS_ActCheckDamage( NPCWORK *npc ) ;
extern int  VMPS_ActCheckPad( NPCWORK *npc ) ;
extern void VMPS_ActDamage( NPCWORK *npc, int time ) ;

/* vamp_msg.c */
extern void VMPS_Message( Work *work ) ;
extern void VMPS_SendMessageFaceAnimePlayMotion( int motion ) ;
extern void VMPS_SendMessageFaceAnimeSightControl( int motion, int type, int time, FVECTOR *pos ) ;

/* vamp_rcg.c */
extern void VMPS_MakeFloor( Work *work, IVECTOR *hzx_pos ) ;
extern void VMPS_Recognize( Work *work ) ;

/* vamp_dsp.c */
extern void VMPS_Display( Work *work ) ;

/* vamp_utl.c */
extern void VMPS_SetAdjustPosition( Work *work, FVECTOR *aim, int flames ) ;
extern int  VMPS_DamageVitality( Work *work, int damage, int pad ) ;
extern int  VMPS_DamageVitalityM9( Work *work, int damage, int pad ) ;
extern int  VMPS_GameOverCheck( Work *work, int pad ) ;
extern void VMPS_CaptureEmma( Work *work ) ;
extern void VMPS_ReleaseEmma( Work *work ) ;

/* npc.c */
extern void NPC_IK_Control( NPCWORK *npc ) ;


/*Reference Global Variable*/
#ifdef __MAIN_FILE__

Work *VMPS_Work = NULL ;
FVECTOR VMPS_Directions[] = {
    { 70125.0f, -37500.0f, -102000.0f,  DEGtoRAD(0)   },
    { 72400.0f, -37500.0f, -102950.0f,  DEGtoRAD(0)   },
    { 69125.0f, -37500.0f, -102600.0f,  DEGtoRAD(0)   },
    { 72400.0f, -37500.0f, -102950.0f,  DEGtoRAD(0)   },
    { 72400.0f, -37500.0f, -102950.0f,  DEGtoRAD(-15) },
    { 70125.0f, -37500.0f, -102000.0f,  DEGtoRAD(-15) },
} ;

#else   /* __MAIN_FILE__ */


extern Work *VMPS_Work ;
extern int VMPS_ChildLevelNum[] ;
extern FVECTOR VMPS_Directions[] ;

#endif  /* __MAIN_FILE__ */


#endif  /* __VMPS_H__ */
