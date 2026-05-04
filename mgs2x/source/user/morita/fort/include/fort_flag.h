/*
   fort_flag.h
   フォーチュン/ヘッダファイル

   2000/07/11 T.Morita
   $Id: fort_flag.h,v 1.1.1.3 2002/11/19 11:46:16 Yoshizawa1 Exp $			
*/

#ifndef __FORT_FLAG__
#define  __FORT_FLAG__

/* フラグ情報(用途複数あり) */
enum flag_t
{
    FRT_F_NONE         = 0x00000000,

    /* work->flag用 */ 
    FRT_F_THINK_MSK    = 0x0000000f, /* 思考（モード＋レベル）   */
    FRT_F_MODE_MSK     = 0x00000001, /* モード（移動0x0／攻撃0x1） */
    FRT_F_OFFENCE      = 0x00000001, /* 攻撃モード（1:攻撃モード 0:移動モード） */
    FRT_F_LEVEL_MSK    = 0x0000000c, /* プレーヤーレベル 0:easy 1:norm 2:hard 3:mania */
    FRT_F_LEVEL_EASY   = 0x00000000,
    FRT_F_LEVEL_NORM   = 0x00000004,
    FRT_F_LEVEL_HARD   = 0x00000008,
    FRT_F_LEVEL_MANIA  = 0x0000000c,

    FRT_F_NVR_DAMAGED  = 0x00000010, /* 無敵                    */
    FRT_F_NVR_RESET    = 0x00000020, /* リセットできない        */
    FRT_F_STOP_STILL   = 0x00000040, /* モーション静止状態      */
    FRT_F_OVERLAYED    = 0x00000080, /* モーションがオーバーレイされた */

    FRT_F_ENB_AIM      = 0x00000100, /* 狙い検知（スネークを狙えるかどうか）*/
    FRT_F_ENB_MOVE_POS = 0x00000200, /* 移動場所変更可能         */
    FRT_F_ENB_ENERGY_SE= 0x00000400, /* エネルギー充填音発生可能 */
    FRT_F_ENB_WATCHATK = 0x00000800, /* 主観直接攻撃が可能       */

    FRT_F_AM_ON_RIGHT  = 0x00001000, /* 右側が危険              */
    FRT_F_AM_ON_LEFT   = 0x00002000, /* 左側が危険              */
    FRT_F_UNRECOG_POS  = 0x00004000, /* 相手が見えないでいる    */
    FRT_F_UNRECOG_AIM  = 0x00008000, /* 相手がどこにいるのか分からない(未使用)  */

    FRT_F_MOVED_ONCE   = 0x00040000, /* 移動した                */
    FRT_F_DONT_SHOOT   = 0x00080000, /* 撃たない                */
    FRT_F_COUNTER_SHOT = 0x00100000, /* カウンターショットを今まで撃ったか  */
    FRT_F_IGNORE_PLYER = 0x00200000, /* プレイヤーを無視する(事実上,未使用) */
    FRT_F_NOMISS_SHOT  = 0x00400000, /* 必ず当てる              */
    FRT_F_AIMMING_OBJ  = 0x00800000, /* 物を狙う                */
    FRT_F_AIMMING_AUTO = 0x01000000, /* 自動的に狙う            */
    FRT_F_SHOOT_AT_ME  = 0x02000000, /* 初めて自分に撃った      */
    FRT_F_THROW_AT_ME  = 0x04000000, /* 初めて自分に投げた      */
    FRT_F_NOMORE_DRUM  = 0x08000000, /* もうドラム缶はない      */

    FRT_F_FACETO_HEAD  = 0x10000000, /* 首のみ回転（首から回転）*/
    FRT_F_FACETO_AIM   = 0x20000000, /* 両手回転（胸回転有り）  */
    FRT_F_FACETO_ALL   = 0x30000000, /* ホーミングフラグすべて  */
    FRT_F_DEMO_MOVIE   = 0x40000000, /* デモの相互排除(再生している時オン) */
    FRT_F_GAME_END     = 0x80000000, /* ゲームが終った          */


    /* その他 */ 
    FRT_F_PLYR_SIGHT   = 0x00008000, /* プレイヤーが主観    (work->ply_sight) */
    FRT_F_PLYR_HIDING  = 0x00008000, /* プレイヤーが隠れてる(work->ply_hide)  */
    FRT_F_PLYR_HIDLNG  = 0x00004000, /* 長い間隠れてる      (work->ply_hide)  */
    FRT_F_PLYR_HIDEXP  = 0x00003000, /* 隠れていない時間    (work->ply_hide)  */
    FRT_F_PLYR_HIDTIC  = 0x00001000, /* 隠れていない時間単位(work->ply_hide)  */
    FRT_F_PLYR_HIDTIM  = 0x0000003f, /* 隠れている時間      (work->ply_hide)  */
    FRT_F_PLYR_HIDPOS  = 0x00000fc0, /* 隠れている場所のID  (work->ply_hide)  */
    FRT_F_PLYR_HIDCLR  = 0x0000ffc0, /* 隠れている時間クリア(work->ply_hide)  */

    /* work->act_flg 特別行動フラグ（ある特定の条件を満たすことでフラグが上がる） */
    FRT_F_STAND_ATTK   = 0x00001000, /* 禁止区域に入っている */
    FRT_F_HIDING_ATTK  = 0x00004000, /* ライデンが安全地帯にいる攻撃 */
    FRT_F_MAGAZIN_ATTK = 0x00008000, /* マガジンに反応する      */
    FRT_F_COUNTER_ATTK = 0x00000001, /* 撃たれた時に撃つ */
    FRT_F_RAGING_ATTK  = 0x00000002, /* じっとしてた時に撃つ */
    FRT_F_GASTANK_ATTK = 0x00000010, /* ガスタンクを撃つ */
    FRT_F_MOVE_ATTK    = 0x00000080, /* 強制移動 */
    FRT_F_INVALID_ATTK = 0x000001e0, /* 特別行動の条件を見ない 制限ビットマスク */
} ;


/* 狙いシステム */
enum
{
    FRT_AIM_EnableSpot    = 0x00000001,
    FRT_AIM_FrontEnable   = 0x00000002,
    FRT_AIM_RearEnable    = 0x00000004,

    FRT_AIM_DontDestinate = 0x00000008,
    FRT_AIM_NoPhase       = 0x00000010,
    FRT_AIM_NoOnlineCheck = 0x00000020,

    FRT_AIM_TypeNothing   = 0x00000100,
    FRT_AIM_TypeHangLight = 0x00000100,
    FRT_AIM_TypeWallLight = 0x00000200,
    FRT_AIM_TypeGasCan    = 0x00000400,
    FRT_AIM_TypeExplosiv_L= 0x00000800,
    FRT_AIM_TypeExplosiv_R= 0x00010000,
    FRT_AIM_TypeForkLift  = 0x00001000,
    FRT_AIM_TypeIronBox   = 0x00002000,
    FRT_AIM_TypeWoodBox   = 0x00004000,
    FRT_AIM_TypeContainer = 0x00008000,
    FRT_AIM_TypeCeiling   = 0x00020000,
} ;


/* プロックID */
enum procs_t
{
    FRT_P_FIRE_AT_FORTUNE=0,//弾避けのカット
    FRT_P_THROW_AT_FORTUNE ,//グレネード系 不発カット
    FRT_P_FORKLIFT_CUT     ,//フォークリフトのカット
    FRT_P_ELEVATOR_CUT     ,//エレベータスイッチのカット
    FRT_P_ELEVATOR_DOWN    ,//エレベータが下がる
    FRT_P_GAME_END         ,//ゲームエンド
    FRT_N_PROCS
} ;

/* メッセージID */
enum message_t
{
    FRT_M_PLYR_GOTOUT = 0,/*   プレイヤーは見えている */
    FRT_M_PLYR_HIDING    ,/*   プレイヤーは隠れている */

    FRT_M_DSP_MUL_WEIGHT ,/* 2 マルチウェイト表示   */
    FRT_M_DSP_SGL_WEIGHT ,/*   シングルウェイト表示 */

    FRT_M_STOPSTILL_START,/* 4 静止開始 リセット掛けて止めてしまう */
    FRT_M_STOPSTILL_END  ,/*   静止止め                            */
    FRT_M_MOVE_DESTINATE ,/* 6 強制移動 位置指定可能               */
    FRT_M_MOVE_TELEPORT  ,/*   瞬間移動 位置指定可能               */

    FRT_M_STOP_ALLACT    ,/* 8 全機能停止フレーム設定              */

    FRT_M_DEMO_SKIP      ,/* 9 デモスキップ可能                    */
    FRT_M_DEMO_START     ,/*   デモ開始                            */
    FRT_M_DEMO_END       ,/*   デモ終了                            */

    FRT_M_SLOWMTN_START  ,/*12 スローモーション開始                */
    FRT_M_SLOWMTN_END    ,/*   スローモーション終了                */

    FRT_M_DBG_ACTION  = 0x000000ff,
} ;


enum start_t
{
    FRT_START_NOBODYSPLASH  = 0x01,
    FRT_START_NOFOOTSHADOW  = 0x02,
    FRT_START_NOBODYSHADOW  = 0x04,
    FRT_START_BODYSPECULAR  = 0x08,
    FRT_START_HAIRMOVE      = 0x10,
} ;

#endif /* __FORT_FLAG__*/
