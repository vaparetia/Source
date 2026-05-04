/*
   orga.h
   オルガ/ヘッダファイル

   1999/12/18 T.Morita
   $Id: orga.h,v 1.1.1.3 2002/11/19 11:46:25 Yoshizawa1 Exp $			
*/
#ifndef __ORGA_H__
#define __ORGA_H__

#include "../../include/util.h"

/* モーション用 */
#include "orga.mh"

/* SE用 */
#include "orga_se.h"

/*狩野さんライブラリ*/
#include "../../../kano/malti_matrix/mm_orga.h"     /* 乳揺れ用 */
#include "../../../kano/attachment/attachments.h"   /* 装備品用 */

/*柴田さんライブラリ*/
#include "../../../shibata/t_headmark/headmark.h"

/* プレイヤー情報用 */
#include "../../../sonoyama/raiden/pl_work.h"
#define	Work_Ply	PlayerWork

#include "orga_flag.h"

#if DEBUG_MODE
/* オルガ プリント消去 */
//#define printf ORG_DummyFunc
#endif

#define ORGA_NAME          GV_StrCode( "オルガ" )
#define ORGA_MOTION_NAME   3757377  /*GV_StrCode( "orga" )   */
#define ORGA_EVMMODEL_NAME 10566064 /*GV_StrCode( "org_def" )*/
#define ORGA_MODEL_NAME    3757377  /*GV_StrCode( "orga" )   */


#define ORGA_MAX_FLOAT 60000000000.0f


/* 思考時間 や その他（思考に関するもの） */
#define ORGA_STILL_THINKTIME 4    /* 攻撃のワンクール（この回数だけ攻撃して移動する） */
#define ORGA_NORMAL_SPEED    64   /* モーションの補間スピードの基準(work->act_speedが利用する) */
#define ORGA_HOLO_THINKTIME  (8*6)/* ホロ隠れ攻撃に際は,この回数のみ行なう */

/*ダメージ関係*/
#define ORGA_DAMAGE_DFLT     32.0f
#define ORGA_MAX_VITALITY    128
#define ORGA_MAX_NONDMG_TIME  8 /*ダメージ受けて難しくなってから易しくなるまでの時間（x4秒）*/
#define ORGA_DFLT_NONDMG_TIME (ORGA_MAX_NONDMG_TIME/4)
#define ORGA_BIT_DAMAGE       4 /* EASY時の減りダメージの速さ */
#define ORGA_BIT_DAMAGE_MIN   (ORGA_MAX_VITALITY/8) /* EASY時の減りダメージの速さ */


#define ORGA_ATTACK_RAD      1400.0f
#define ORGA_WATCH_RAD       1500.0f 

/* 壁当たりチェック半径 や 隠れ関係 */
#define ORGA_CHECK_SPHERE    500
#define ORGA_NORMAL_SPHERE   300

#define ORGA_HIDE_V_SPHERE     850 /* 障害物に隠れる半径                   */
#define ORGA_HIDE_B_SPHERE     550 /* 障害物にビハンド時に隠れる時の半径   */
#define ORGA_HIDE_H_SPHERE     300 /* 障害物に隠れる時の横の半径           */
#define ORGA_HIDE_MOV_SPHERE   550 /* 障害物からどれだけ離れて移動するか   */
#define ORGA_HIDE_CHK_SPHERE   250 /* 移動時の障害物衝突判定の最小マージン */
#define ORGA_HIDE_SQUAT_SPHERE 50

#define ORGA_DANGER_HEIGHT  1200       
#define ORGA_HIDE_RADIUS    (400*400)

/*移動完了距離 また 移行距離 */
#define ORGA_TUMBLE_DIST  (4000.0f*4000.0f)
#define ORGA_SLIDING_DIST (3400.0f*3400.0f)
#define ORGA_RUN_DIST     ( 800.0f* 800.0f)
#define ORGA_WALK_DIST    ( 300.0f* 300.0f)
#define ORGA_STOP_DIST    3000.0f 

#define ORGA_VOX_INTREVAL   8 /* セリフの間の秒数 */

/* ミスショットする 範囲 */
#define ORGA_MISS_SHOT_RAD  500.0f
/* マガジンなどの音がする 範囲 */
#define ORGA_HEARD_RAD     (4000.0f*4000.0f)

/* ターゲット関連 */
#define ORGA_PART_TRG_FLG (TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|TARGET_ONLINE|TARGET_ROTATE)
#define ORGA_MAIN_TRG_FLG (TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|TARGET_ONLINE|TARGET_CHILD)
#define ORGA_N_LVL0_PARTS 2
#define ORGA_N_LVL1_PARTS 8
#define ORGA_N_LVL2_PARTS 1
#define ORGA_N_PARTS      (ORGA_N_LVL0_PARTS + ORGA_N_LVL1_PARTS + ORGA_N_LVL2_PARTS)

#define ORGA_N_ACT_HIST    8  /* 行動を記憶するヒストリー（同じ行動を起こさないように 2の累乗） */
#define ORGA_N_HIDE_HIST   2  /* 隠れた場所のヒストリー */
#define ORGA_N_HIDE_POOL   10 /*  */
#define ORGA_N_HIDE_DFLT   3 /*  */
#define ORGA_N_ROUTE_HIST  10 /* 移動用のターゲットヒストリー */
#define ORGA_N_STREAM      6 /* ストリーミング */
 

/* オブジェクト部位フラグ */
#define ORGA_BODY_UPPER     ( (1<<HUMAN21_ONAKA)| (1<<HUMAN21_MUNE)| \
			      (1<<HUMAN21_KUBI)| (1<<HUMAN21_ATAMA)| \
			      (1<<HUMAN21_MIGI_KATA  )|(1<<HUMAN21_MIGI_UDE1  )| \
			      (1<<HUMAN21_MIGI_UDE2  )|(1<<HUMAN21_MIGI_TE    )| \
			      (1<<HUMAN21_HIDARI_KATA)|(1<<HUMAN21_HIDARI_UDE1)| \
			      (1<<HUMAN21_HIDARI_UDE2)|(1<<HUMAN21_HIDARI_TE  ) )
#define ORGA_BODY_RIGHT_ARM ( (1<<HUMAN21_MIGI_KATA  )|(1<<HUMAN21_MIGI_UDE1  )| \
			      (1<<HUMAN21_MIGI_UDE2  )|(1<<HUMAN21_MIGI_TE    ) )
#define ORGA_BODY_BOTH_ARMS ( (1<<HUMAN21_MIGI_KATA  )|(1<<HUMAN21_MIGI_UDE1  )| \
			      (1<<HUMAN21_MIGI_UDE2  )|(1<<HUMAN21_MIGI_TE    )| \
			      (1<<HUMAN21_HIDARI_KATA)|(1<<HUMAN21_HIDARI_UDE1)| \
			      (1<<HUMAN21_HIDARI_UDE2)|(1<<HUMAN21_HIDARI_TE  ) )
#define ORGA_BODY_LOWER     ( (1<<HUMAN21_KOSHI)| \
			      (1<<HUMAN21_MIGI_ASHI1   )|(1<<HUMAN21_MIGI_ASHI2    )| \
			      (1<<HUMAN21_MIGI_KAKATO  )|(1<<HUMAN21_MIGI_TSUMASAKI)| \
			      (1<<HUMAN21_HIDARI_ASHI1 )|(1<<HUMAN21_HIDARI_ASHI2  )| \
			      (1<<HUMAN21_HIDARI_KAKATO)|(1<<HUMAN21_HIDARI_TSUMASAKI) )
#define ORGA_BODY_ALL         (ORGA_BODY_UPPER | ORGA_BODY_LOWER)

/* ターゲット部位フラグ */
#define ORGA_AVOID_RIGHT  0x80000000
#define ORGA_AVOID_LEFT   0x40000000
#define ORGA_AVOID_UPPER  0x00000417
#define ORGA_AVOID_LOWER  (~(ORGA_AVOID_UPPER| ORGA_AVOID_RIGHT| ORGA_AVOID_LEFT))  

/* 狙う間接数の数(ORG_AimJoint[]の要素数) */
#define ORGA_N_AIM_JOINT    5

/* ミスショットの時に狙う場所(ORG_DummyPos[]の要素数) */
#define ORG_N_DUMMYPOS      16

/* グレネード投げ場所(ORG_SnakeHidingPos[]の要素数) */
#define ORG_N_SNAKE_HIDEPOS 6

/* USP弾数 */
#define ORGA_WEAPON_BULLET  15
#define ORGA_DSE_BULLET     7


/* プレイヤー情報 */
#define ORGA_PLY_VITALITY   (GM_Vitality        )
#define ORGA_PLY_REFTRATION (GM_ItemNum(IT_Ration))
#define ORGA_PLY_REFTAMMO   (GM_WeaponNum(WP_m92) )
#define ORGA_PLY_CAMERAPOS  (GM_PlayerSubjectCamera[0]->position)
#define ORGA_PLY_SHOOT      (GM_WeaponFire==WP_m92 || GM_WeaponFire==WP_Usp)

#define ORGA_RESET_PLY_POSITION  -1
#define ORGA_GET_PLY_CONTROL     21
#define ORGA_GET_PLY_OFFSET      22
#define ORGA_PLY_MAX_ENTRIES     23

/* マクロ関数 */
#define BODYWORLD(a,b) ((a)->objs->objs[(b)].world)
#define BODYPOS(a,b)   ((FVECTOR*)&((a)->objs->objs[(b)].world.m[W]))
#define N_MOTION(l) (body.m_ctrl->mt3_ctrl[(l)].motion_num)

#define CHECKACT(_w) ((_w)->act_flg & 0xffff)

/* 角度関係 */
#define DEG2RAD(_s) (float)((_s)*(float)M_PI/180.0f)
#define RAD2ANG(_s) (short)((_s)*2048.0f/(float)M_PI)
#define ANG2RAD(_s) (float)((_s)*(float)M_PI/2048.0f)


/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

enum start_t
{
    ORGA_START_NOBODYSPLASH  = 0x01,
    ORGA_START_NOFOOTSHADOW  = 0x02,
    ORGA_START_NOBODYSHADOW  = 0x04,
    ORGA_START_BODYSPECULAR  = 0x08,
    ORGA_START_HAIRMOVE      = 0x10,
    ORGA_START_MERYLE        = 0x20,
} ;


/* プロックID */
enum procs_t
{
    ORGA_P_HOLO_FIRE=0,// ホロ撃ち（必須）  ホロを飛ばすために隠れて撃つ時
    ORGA_P_PASTOUT,    // 死に（必須）      オルガを倒した
    ORGA_P_GRENADE,    // グレネードなげ    グレネードをスネークに向かって投げる時
    ORGA_P_BLIND_FIRE, // めくら撃ち 乱射   物陰からスネークを見ずに乱射する時
    ORGA_P_ANTI_ERUDE, // アンチエルード    スネークがエルードした時に撃つ
    ORGA_P_AIR_JUMP,   // とんぼ返り        側転から狙われたときに出る発動する
    ORGA_P_SIDE_JUMP,  // 側転              ダメージを多く受けた時に側転をして移動し始める
    ORGA_P_DAMAGED,    // ダメージ          ダメージを受けた時
    ORGA_P_PEEP,       // 覗き込み          警戒している覗き込みをしたとき
    ORGA_P_SLIDING,    // スライディング    スライディングをするとき
    ORGA_P_MACHINGUN,  // マシンガンな奴    ステップ前に入ってきたときに撃ちまくる
    ORGA_P_TEASE,      // 挑発              止まっている時間が長く,低い隠れ場所にいるときする
    ORGA_P_HOLO_GONE,  // ホロ舞う          ホロが飛び去る何れかのときに必ず呼ばれる
    ORGA_P_LIFE_HALF,  // ライフ１／２      ライフが１／２になったときに呼ばれる
    ORGA_P_LIFE_QUAD,  // ライフ１／４      ライフが１／４になったときに呼ばれる
    ORGA_P_LIFE_EIGHT, // ライフ１／８      ライフが１／８になったときに呼ばれる
    ORGA_P_LIFE_THIRD, // ライフ３／４      ライフが３／４になったときに呼ばれる
    ORGA_P_SNAKE_HIDE, // スネークのすき間攻撃
    ORGA_P_BIBLIO_FST0, // 語りのセリフ最初 始め
    ORGA_P_BIBLIO_FST1, // 語りのセリフ最初 終り
    ORGA_P_BIBLIO_LST0, // 語りのセリフ最後 始め
    ORGA_P_BIBLIO_LST1, // 語りのセリフ最後 終り
    ORGA_P_HOLO_MUST,  // ホロ舞う          ホロが飛び去る何れかのときに必ず呼ばれる
    ORGA_P_FLARED_LIGHT, // 投光器を向ける
    ORGA_P_FLARED_END  , // 投光器攻撃終り

    ORGA_N_PROCS
} ;

/* メッセージID */
enum message_t
{
    ORGA_M_PLYR_GOTOUT = 0,/*   プレイヤーは見えている */
    ORGA_M_PLYR_HIDING    ,/*   プレイヤーは隠れている */
    ORGA_M_HOLO_GONE      ,/* 2 ホロは吹き飛んだ       */
    ORGA_M_SNA_BARR_IN    ,/*   立ち入り禁止区域に入ってきた */
    ORGA_M_SNA_BARR_OUT   ,/* 4 立ち入り禁止区域から出てった */
    ORGA_M_SNA_HIDE_TARGET,/*   スネーク隠れ場所用ターゲット（現在未使用）*/

    ORGA_M_DSP_MUL_WEIGHT ,/* 6 マルチウェイト表示   */
    ORGA_M_DSP_SGL_WEIGHT ,/*   シングルウェイト表示 */

    ORGA_M_STOPSTILL_START,/* 8 静止開始 リセット掛けて止めてしまう */
    ORGA_M_STOPSTILL_END  ,/*   静止止め                            */
    ORGA_M_MOVE_DESTINATE ,/*10 強制移動 位置指定可能               */
    ORGA_M_MOVE_TELEPORT  ,/*   瞬間移動 位置指定可能               */

    ORGA_M_AVOID_CANCEL   ,/*12 回避キャンセル                      */
    ORGA_M_AVOID_ENABLE   ,
    ORGA_M_STOP_ALLACT    ,/*14 全機能停止フレーム設定              */

    ORGA_M_DEMO_SKIP      ,/*15 デモスキップ可能                    */
    ORGA_M_DEMO_START     ,/*   デモ開始                            */
    ORGA_M_DEMO_END       ,/*   デモ終了                            */

    ORGA_M_FLARED_END     ,/*18 第3フェーズの終了メッセージ         */
    ORGA_M_FLARED_CHANGE  ,/*   投光器の方向を変える願い            */

    ORGA_M_DBG_ACTION  = 0x000000ff,
} ;

/* 動作可能領域(オルガの移動可能範囲) */
#define MOVABLE_AREA1 {-12000,0,-20000,0}
#define MOVABLE_AREA2 {-20000,0,-20000,0}
#define MOVABLE_AREA3 {-20000,0,-14000,0}
#define MOVABLE_AREA4 {-12000,0,-12000,0}


/* 型定義 */
typedef	struct work_t Work ;
typedef struct part_t PART ;
typedef enum   flag_t FLAG ;
typedef union  args_t ARGS ;

struct part_t
{
    FMATRIX *world   ; /* 間接のワールド（DG_OBJのworld） */

    Work    *work    ; /* コールバック用のワークへのポインタ */
    int      part_id ; /* 間接番号(BODY21_?????) */
    int      damage  ; /* ダメージ量             */
    int      motion  ; /* ダメージモーション番号 */
} ;

typedef struct hide_t HIDE ;
struct hide_t
{
    float    p[4][XY] ;  /* 四隅位置（移動用） */

    FVECTOR  left   ;    /* 左の隠れ位置（隠れ位置計算用） */
    FVECTOR  right  ;    /* 右の隠れ位置（隠れ位置計算用） */
    FVECTOR  center ;    /* 中心位置*/
    FVECTOR  size   ;    /* 大きさ（中心からの幅や高さ） */

    DG_OBJS *objs   ;    /* 障害物のオブジェ */

    float    height ;    /* 障害物の高さ （オブジェクトの高さ）*/
    short    turn   ;    /* ビハインド用の壁向き角度(ビハインドや前向きの角度を決める) */
    short    id     ;    /* ハイドのID */
} ;



union  args_t
{
    char     c ;
    u_char   uc ;
    int      i ;
    u_int    ui ;
    short    s ;
    u_short  us ;
    float    f ;

    FMATRIX *m ;
    FVECTOR *v ;

    void    *a ;
} ;

struct work_t
{
    GV_ACT	 actor     ;

    /* システム用データ */
    CONTROL	 control   ;	/* 移動制御 */
    OBJECT	 body      ;	/* モデル   */
    FMATRIX	 lights[2] ;	/* ライト   */
    RADAR_CTRL   radar     ;    /* レーダー */
    GM_GageSet	 gage      ;    /* ライフゲージ */

    HOMING_TRG   homing    ;              /* ホーミング部位ターゲット(現在 BODY21_ATAMA) */
    TARGET       target[ORGA_N_PARTS+1] ; /* 防御 部位ターゲット                      */
    POWER_TARGET power [ORGA_N_PARTS+1] ;
    PART         parts [ORGA_N_PARTS  ] ; /* 部位情報(ダメージ量やモーション番号など) */
    int          name      ;              /* メッセージ受取用                         */
    int          procs [ORGA_N_PROCS  ] ; /* プロックリスト                           */
    int          str_id[ORGA_N_STREAM ] ; /* ファイルストリーム用                     */
    int          str_hdl   ;              /* ストリームハンドラー  */
    int          vib_time  ;              /* 振動センサー用 GV_Time */

    FVECTOR      pos_adj   ;              /* 位置のアジャスト値(フレーム毎加算)       */

    FVECTOR      head      ; /* 間接のアジャスト値(Eular角/ラジアン) */
    FVECTOR      arm_l     ; /* 間接のアジャスト値(Eular角/ラジアン) */
    FVECTOR      arm_r     ; /* 間接のアジャスト値(Eular角/ラジアン) */
    FVECTOR      breast    ; /* 間接のアジャスト値(Eular角/ラジアン) */

    /* オルガ 専用 パラメータ */
    FLAG         flag      ;    /* オルガのステータスフラグ */
    FLAG         act_flg   ;    /* 特別行動フラグ */
    u_short      avoid_flg ;    /* 回避（ターゲット）部位フラグ */
    u_short      aim_flg   ;    /* 狙撃（オブジェクト）部位フラグ */

    short        scared    ;    /* 恐怖 */
    short        rage      ;    /* 怒り */
    float        speed     ;    /* 移動スピード */

    void      (**act)( Work * ) ;            /* 現在の行動   */
    u_int        act_hist[ORGA_N_ACT_HIST] ; /* 行動履歴     */
    char         act_h_idx ;                 /* 行動履歴インデックス */
    char         act_tic  ;                  /* 行動TIC(PAL:5/NTSC:6) */
    short        act_time ;                  /* 行動時間(無限行動禁止)  */
    u_char       act_speed ;                 /* 行動のあせり(64 が ORGA_NORMAL_SPEED) */
    u_char       act_stop  ;                 /* 全てアクトを通らない残りフレーム数（全機能停止） */

    char         stll_mtn  ;  	/* 静止モーション */
    char         stll_tim  ;  	/* 静止モーション 思考時間 */
    short        wait_sec  ;  	/* 静止タイム用 */
    short        vitality     ;    /* 命 */
    short        vitality_m9  ;    /* 命 */

    short        weap_ssk  ;  	/* ナイフ(SSK)の引金  */
    short        weap_blt  ;  	/* 銃(USP)の残弾  */
    short        weap_max  ;  	/* 銃(USP)の弾ＭＡＸ数 */
    short        weap_amo  ;    /* USPのマガジン  */
    FVECTOR      weap_usp  ;  	/* 銃    (USP)の引金  */
    FVECTOR      weap_sgr  ;  	/* 手榴弾(SGR)の引金  */
    FVECTOR     *trgt_eye  ;    /* 視線先 */
    FVECTOR     *trgt_aim  ;    /* 狙い先 */
    FVECTOR      trgt_aim_pos ; /* 狙い先コントロール用 */
    FVECTOR      trgt_hid  ;    /* 隠れているハイド */
    FVECTOR      trgt_frc  ;    /* 移動 強制移動先 */
    FVECTOR      trgt_nse  ;    /* 音の発振源 */
    FVECTOR     *trgt_pos  ;    /* 移動先 */
    FVECTOR     *trgt_nxt  ;    /* 移動中に遮られた時,次にいくべきルート */
    u_int        trgt_his[ORGA_N_ROUTE_HIST] ;    /* ルートの履歴 */
    u_int        n_trgt_his ;   /* ルートの履歴数 */
    short        voice     ;    /* 声のSE番号 */
    short        voice_vox ;    /* 声のSTREAM番号 */
    short        voice_tim ;    /* 声が出るまでのフレーム時間 */
    short        head_time ;    /* ヘッドマークを出すタイミングを計るため */
    int          head_mark ;    /* ヘッドマークを出す */
    short        non_dmg   ;    /* ダメージを受けてからの時間 */
    short        bit_dmg   ;    /* 麻酔で減る体力の量 */

    /* ハイド情報 */
    HIDE        *hide_hist[ORGA_N_HIDE_HIST] ; /* ハイドの履歴 */
    HIDE        *hide_pool ;    /* ハイドの配列の先頭アドレス  */
    int          n_hide_pool ;  /* 有効なハイドの数            */
    HIDE        *hide_spot ;    /* 現在隠れているハイド        */
    int          hide_stat ;    /* 現在のハイドできる状態（オルガの行動に深く関係する）*/
    short        exposed   ;    /* 隠れからはみ出ている時間    */
    TARGET      *hide_trgt ;    /* スネークの隠れターゲット */

    /* プレイヤー情報 */
    u_short      ply_shoot  ;    /* プレイヤーの狙わず撃った回数 */
    u_short      ply_time   ;    /* プレイ時間( 30sec as one ) */
    u_short      ply_hide   ;    /* 隠れている (1[flg]:1[flg]:2[expose]:6[place]:6[time]) */
    u_short      ply_stop   ;    /* プレイヤーが止まっている   */
    u_short      ply_locate ;    /* プレイヤーのいる大まかな場所 */

    /* その他の情報 */
    int         *misc_holo_stat  ; /* ホロの状態       */
    int         *misc_spot_stat  ; /* 投光器の状態     */
    void        *misc_breast     ; /* 乳揺れ用ワークメモリ */
#if 0
    int         *misc_n_potato1  ; /* ポテトの数       （未使用）*/
    int         *misc_box_vital1 ; /* ポテトの箱の寿命 （未使用）*/
    FVECTOR     *misc_box_pos1   ; /* ポテトの箱の位置 （未使用）*/
    int         *misc_n_potato2  ; /* ポテトの数       （未使用）*/
    int         *misc_box_vital2 ; /* ポテトの箱の寿命 （未使用）*/
    FVECTOR     *misc_box_pos2   ; /* ポテトの箱の位置 （未使用）*/
#endif

#if DEBUG_MODE
    /* DEBUG用 リスタートの初期化用 */
    FVECTOR      init_pos ;
    SVECTOR      init_rot ;
#endif

} ;


/*inline functions*/
static inline float VectorSquare( FVECTOR *a, FVECTOR *b )
{
    return (a->vx-b->vx)*(a->vx-b->vx) + (a->vz-b->vz)*(a->vz-b->vz) ;
}
static inline float BackFaceCullingXZ( FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    return (v1->vx - v2->vx)*(v3->vz - v2->vz) - (v1->vz - v2->vz)*(v3->vx - v2->vx) ;
}
static inline int ORG_RecalcDir( int r )
{
    r &= 4095 ;
    r -= r>2048 ? 4096 : 0 ;
    return r ;
}


/* external variable */
extern CONTROL  *GM_PlayerControl  ;
extern OBJECT   *GM_PlayerBody     ;
extern OBJECT   *GM_PlayerArmBody  ;
extern FVECTOR   GM_PlayerPosition ;
extern Work_Ply *GM_PlayerWork     ;
extern HIDE ORG_HideBranch[] ;/* オルガ側障害物のハイドデータ */
extern HIDE ORG_HoloHide ;    /* ホロ開き時用のハイドデータ */

extern GM_CameraSet *GM_PlayerSubjectCamera[ 4 ] ;	/* プレイヤー主観カメラ */


/* Prototypes */
/* org_ini.c */
extern int  ORG_InitControl( Work *work, int name, int where ) ;
extern int  ORG_InitObject( Work *work ) ;
extern int  ORG_InitParam( Work *work, int name ) ;
extern int  ORG_GetOptionValue( Work *work, int name ) ;
extern int  ORG_InitPartAndTarget( Work *work, int map ) ;
extern int  ORG_InitVitalityGage( Work *work ) ;
extern void ORG_InitMessages( Work *work ) ;

extern int  ORG_InitRadar( Work *work ) ;

/* org_act.c */
extern void ORG_PreAction( Work *work ) ;
extern void ORG_PostAction( Work *work ) ;
extern void ORG_ActUnrecogFireP( Work *work ) ;
extern void ORG_ActFireP( Work *work ) ;

extern void ORG_ActDamageStart( Work *work, int motion ) ;
extern void ORG_ActDamageOverStart( Work *work ) ;
extern void ORG_ActStillStart( Work *work ) ;
extern void ORG_ActThink( Work *work ) ;


/* org_lst.h */
extern void (*ORG_ActionDefault0[])( Work * ) ;
extern void (*ORG_ActionDefault1[])( Work * ) ;
extern void (*ORG_ActionAbort[])( Work * ) ;
extern void (**ORG_ActListWait[])( Work * ) ;
extern void (*ORG_ActionReset[])( Work * ) ;
extern void (*ORG_ActionStopStill[])( Work * ) ;
extern void (*ORG_ActionSnipeSnake[])( Work * ) ;


/* org_clb.c */
extern void ORG_DamageVitality( Work *work, int damage, u_long64 m92 ) ;
extern void ORG_PartsTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;
extern void ORG_MainTargetCallBack( TARGET *off, TARGET *def, void *ptr ) ;


/* org_dsp.c */
extern void ORG_Display( Work *work ) ;


/* org_rcg.c */
extern void     ORG_RecogStartStream( Work *work ) ;
extern void     ORG_RecogStopStream( Work *work ) ;
extern FVECTOR *ORG_RecogAimPos( Work *work ) ;
extern FVECTOR *ORG_RecogPlayerPos( int joint ) ;
extern FVECTOR *ORG_RecogPlayerPosNoHide( int joint ) ;
extern void     ORG_CalcHidePos( FVECTOR *pos ) ;
extern float    ORG_CalcNewHidePosZ( float *p1, float *p2, FVECTOR *pp, float h, float x ) ;
extern float    ORG_GetHidePosZ( HIDE *hide, FVECTOR *pp, float h, int stll_mtn ) ;
extern int      ORG_RenewHidePosZ( HIDE *hide, FVECTOR *pp, int stll_mtn ) ;
extern void     ORG_Recognition( Work *work ) ;

extern int      ORG_SearchHidePlace( Work *work, int stll_mtn ) ;
extern int      ORG_RecogCheckHoloValid( Work *work ) ;
extern int      ORG_RecogCheckSpotLight( Work *work ) ;


/* org_msg.c */
extern void ORG_Message( Work *work ) ;

extern void ORG_SendHoloMessage( int msg ) ;               /* misc/orga_hol.c へのメッセージ */
extern void ORG_SendHoloGetFlagAddrMessage( int **flag ) ; /* misc/orga_hol.c へのメッセージ */
extern void ORG_SendGetNearBoxMessage( FVECTOR *from, FVECTOR *to, FVECTOR *r ) ;
extern void ORG_SendWallScarRandomSEMessage() ;            /* morita/wall_scar/ へのメッセージ */
extern void ORG_SendSpotLightGetFlagAddrMessage( int **flag ) ; /* misc/orga_lgt.c へのメッセージ */
extern void ORG_SendDebugDumpMessage() ; /* デバッグ状態を表示する */
extern void ORG_SendSpotLightFlareMessage() ;

/* org_dbg.c */
extern void ORG_Debug( Work *work ) ;
extern void ORG_DummyFunc( char *fmt, ... ) ;


/* external CHARA */
extern void *NewOrgaWeaponUSP( FMATRIX *waist, FMATRIX *right, FMATRIX *left,
			       FVECTOR *trig, short* bul, int damage, int model ) ;  /* weapon/orga_usp.c */
extern void *NewOrgaWeaponSSK( FMATRIX *pos ) ;                           /* weapon/orga_ssk.c */
extern void *NewOrgaWeaponSGR( FMATRIX *pos, FVECTOR *trig,
			       int damage, int dmg_rad, int blw_rad ) ;   /* weapon/orga_sgr.c */
extern void *NewOrgaWeaponUSPAMO( FMATRIX *r, FMATRIX *l, short *flag, int model ) ; /* weapon/orga_amo.c */
extern void *NewOrgaHair( FMATRIX *world, int id, FMATRIX *lgt ) ;        /* misc/orga_har.c   */
extern void *NewOrgaWeaponBarriar( int mode, FVECTOR *orga ) ;            /* weapon/orga_bar.c */

/* tanaka/effect/             */
extern void *AN_HeadMark( FMATRIX *world, int mode ) ;
/* for test                   */
extern void *AN_Test_Eye2( FVECTOR *mov, int size ) ;
/* tanaka/piyori/             */
extern void *NewPiyori( FMATRIX *world ) ;
/* okajima/effect/body_sdw.c  */
extern void *NewBodyShadow( DG_OBJS *o ) ;
/* misc/orga_sph.c            */
extern void *NewOrgaBodySplash( OBJECT *body, CONTROL *c ) ;
/* okajima/effect/ft_splash.c */
extern void *NewFootSplash( OBJECT *body, CONTROL *c ) ;
/* okajima/effect/body_sph.c  */
extern void *NewBodySplash2( DG_OBJS *o, CONTROL *c, int model_id, int *flag ) ;

/* morita/shadow/foot_shadow.c  */
extern void *NewShadow( DG_OBJ *lfoot, DG_OBJ *rfoot, CONTROL *c, FMATRIX *lgt, int *flag );
/* morita/shadow/drop_shadow.c  */
extern void *NewDropShadow( OBJECT *body, CONTROL *c, FMATRIX *lgt, int *flag ) ;

/* okajima/demo_effect/d_splash_parts.c*/
extern void *NewSplashParts_Demo( FVECTOR *center, SVECTOR *rot, float intense ) ;
/* shibata/t_headmark/control_headmark2.c */
extern void *NewControl_Headmark2( FMATRIX *world, int *type, TARGET *trg, CONTROL *cntrl ) ;

extern void *NewPadVibration( char *scr, int type ) ;

/* sonoyama/etc/gaged.c prototype-set */
extern void GM_AppendGageSet( GM_GageSet *gs ) ;
extern void GM_RemoveGageSet( GM_GageSet *gs ) ;
extern void GM_SetGageColor( GM_GageSet *gs,
			     u_char r1, u_char g1, u_char b1,
			     u_char r2, u_char g2, u_char b2,
			     u_char r3, u_char g3, u_char b3,
			     u_char r4, u_char g4, u_char b4 ) ;
extern void GM_VisibleGage( GM_GageSet *gs ) ;
extern void GM_InvisibleGage( GM_GageSet *gs ) ;
extern void GM_InitGageSet( GM_GageSet *gs, char *name,
			    int x, int w, int h, int value,
			    int max, int min, int delay, int level ) ;

/* kano/attachment/attachments.c*/
extern void *NewAttachments_called( OBJECT *target,
				    const ATTACHMENT_ARGUMENT  *args1, int size1,
				    const ATTACHMENT_ARGUMENT2 *args2, int size2,
				    const ATTACHMENT_ARGUMENT3 *args3, int size3 ) ;

/* shibata/effect/lens_flr_gm.c*/
extern void *NewLensFlr_Game( FVECTOR *center, float size, int max_alpha ) ;

/* holo/orga_hol_msg.c */
extern void ORG_HOL_GetStringPos( FVECTOR *a ) ;




#if DEBUG_MODE
//for debug
#include "../../include/util.h"
#endif



/*Reference Global Variable*/

#ifdef __MAIN_FILE__

int ORG_AimJoint[] =
{
    HUMAN21_MIGI_UDE2,
    HUMAN21_HIDARI_UDE2,
    HUMAN21_MIGI_ASHI2,
    HUMAN21_HIDARI_ASHI2,
    HUMAN21_KUBI,
} ;

FVECTOR ORG_ShootErudePos = { -15600, 13600, -13500, 0 } ;/* エルードを撃ち落とす */
FVECTOR ORG_ShootHoloPos  = { -19000, 13855, -14600, 0 } ;/* ホロを撃つポジション */
FVECTOR ORG_ShootHoloPos2 = { -18500, 13855, -18900, 0 } ;/* 撃たれた時隠れてホロを撃つポジション */
FVECTOR ORG_ShootHoloStr1 = { -18000, 13000, -15850, 0 } ;/* ホロ オルガ側 */
FVECTOR ORG_ShootHoloStr2 = { -18000, 13000, -18000, 0 } ;
FVECTOR ORG_ShootHoloStr3 = { -16000, 12870, -18130, 0 } ;/* ホロ スネーク側 */
FVECTOR ORG_ShootHoloStr4 = { -16000, 12870, -15850, 0 } ;
FVECTOR ORG_ShootSpotLgt  = { -20123, 14700, -15300, 0 } ;/* ライトを撃つポジション */

FVECTOR ORG_Ply_SquatPos  = { -11500, 13000, -16120, 0 } ;
FVECTOR ORG_Ply_AttkPosL  = { -15180, 13000, -15920, 0 } ;
FVECTOR ORG_Ply_AttkPosR  = { -15180, 13000, -17865, 0 } ;
FVECTOR ORG_Ply_AttkGrnd  = { -13100, 13000, -19400, 0 } ;/* 手前へ来過ぎた時の位置 */
FVECTOR ORG_Ply_AttkStnd  = { -17700, 13000, -19300, 0 } ;
FVECTOR ORG_Ply_AttkHoloL = { -18900, 13000, -14200, 0 } ;/* ホロ攻撃時の位置 */
FVECTOR ORG_Ply_AttkHoloR = { -18900, 13000, -15600, 0 } ;/* ホロ攻撃時の位置 */
FVECTOR ORG_Ply_AttkLightL = { -18900, 13000, -14200, 0 } ;/* ライト攻撃時の位置 */
FVECTOR ORG_Ply_AttkLightR = { -18900, 13000, -15600, 0 } ;/* ライト攻撃時の位置 */
FVECTOR ORG_Ply_AttkHoloBlow = { -14250, 13000, -14289, 0 } ;/* 前側のホロを飛ばす */

FVECTOR ORG_Ply_AttkBib[] = {
    { -19500, 13855, -18000, 0 },/* ホロ後ろ */
    { -14250, 13000, -14289, 0 },
    { -18900, 13000, -15600, 0 },/* ホロ後ろ */
    { -14250, 13000, -14289, 0 },

    { -14250, 13000, -14289, 0 },
    { -14250, 13000, -14289, 0 },
} ;

FVECTOR ORG_DummyPos[] = {
    {-10000,13100-50,-15600 }, { -9800,13100-50,-15000 }, { -9750,13100-50,-14300 },
    { -9700,13100-50,-14700 }, {-10300,13100-50,-15800 }, { -9650,13100-50,-15500 },
    {-10382,13618-50,-16375 }, {-10037,13658-50,-16267 },
    { -9900,13100-50,-19088 }, { -9680,13100-50,-19040 }, {-10500,13100-50,-19217 },
    {-11487,13100-50,-12147 }, {-11255,13200-50,-12080 }, {-11255,13490-50,-12040 },
    { -9511,13100-50,-12070 }, { -9864,13100-50,-12292 },
} ;

FVECTOR ORG_SnakeHidingPos[] = {
    {-10893,13500,-10912 }, { -9000,13500,-11574 }, { -9000,13500,-15129 }, { -9000,13500,-16500 },
    { -9000,13500,-18000 }, { -9000,13500,-19000 },
} ;
FVECTOR ORG_ShootHoloRun1 = {-14520,13500,-14222 } ;
FVECTOR ORG_ShootHoloRun2 = {-14182,13892,-18192 } ;

FVECTOR ORG_SnakeFBTNArea = { -10979.0f, 13027.0f, -13713.0f, 0.0f } ;
FVECTOR ORG_HereIam ;

Work *ORG_Work = NULL ;

#else   /* __MAIN_FILE__ */

extern int ORG_AimJoint[] ;

extern FVECTOR ORG_ShootErudePos ;
extern FVECTOR ORG_ShootHoloPos  ;
extern FVECTOR ORG_ShootHoloPos2 ;
extern FVECTOR ORG_ShootHoloStr1, ORG_ShootHoloStr2 ;
extern FVECTOR ORG_ShootHoloStr3, ORG_ShootHoloStr4 ;
extern FVECTOR ORG_ShootSpotLgt  ;

extern FVECTOR ORG_Ply_SquatPos  ;
extern FVECTOR ORG_Ply_AttkPosL  , ORG_Ply_AttkPosR  ;
extern FVECTOR ORG_Ply_AttkGrnd  , ORG_Ply_AttkStnd  ;
extern FVECTOR ORG_Ply_AttkHoloL , ORG_Ply_AttkHoloR  ;
extern FVECTOR ORG_Ply_AttkLightL, ORG_Ply_AttkLightR ;
extern FVECTOR ORG_Ply_AttkHoloBlow ;

extern FVECTOR ORG_ShootHoloRun1, ORG_ShootHoloRun2 ;

extern FVECTOR ORG_Ply_AttkBib[] ;

extern FVECTOR ORG_DummyPos[] ;
extern FVECTOR ORG_SnakeHidingPos[] ;
extern FVECTOR ORG_SnakeFBTNArea ;

extern FVECTOR ORG_HereIam ;

extern Work *ORG_Work ;

#endif  /* __MAIN_FILE__ */

#endif  /* __ORGA_H__ */


