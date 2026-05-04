/*
  ema_flag.h
  エマフラグ ヘッダ

  2001/02/08 Y.Korekado
  $Id: emma_flag.h,v 1.1.1.3 2002/11/19 11:46:04 Yoshizawa1 Exp $
*/

#ifndef __EMA_FLAG_H__
#define __EMA_FLAG_H__ 1

/* アクションフラグ */
enum emma_t
{
    EMA_F_NONE         = 0x00000000,

    EMA_F_NON_DAMAGE   = 0x00000001, /* エマは,無敵状態（毎フレームクリアされる） */
    EMA_F_NON_FACING   = 0x00000002, /* エマは,顔向けしない状態（毎フレームクリアされる） */
    EMA_F_NON_ACTION   = 0x00000004, /* エマの活動を止める（モデルも非表示） */
    EMA_F_NON_MOVE     = 0x00000008, /* エマの動きを止める */

    EMA_F_RESET_CONTROL= 0x00000020, /* コントロールのハザード関係をリセットする action/act.hで */


    EMA_F_IS_COMMING   = 0x00000010, /* 禁止領域から出ている */
    EMA_F_IS_ON_STEP   = 0x00000040,
    EMA_F_IS_ON_LEFT   = 0x00000080, /* ライデンの左側にいる */
    EMA_F_IS_ON_BRIDGE = 0x00000100, /* 桟橋の上にいる          (狙撃イベントのみ) */
    EMA_F_IS_ATTACKED  = 0x00000200, /* 誰かから攻撃を受けている(狙撃イベントのみ) */
    EMA_F_IS_RECOVER   = 0x00000400, /* 半分まで回復する        (狙撃イベントのみ) */


    EMA_F_ENB_LINK     = 0x00000800, /* 手繋ぎ可能              (手繋ぎイベントのみ) */
    EMA_F_ENB_GO_UP    = 0x00001000, /* 上がる準備が出来た      (狙撃イベントのみ) */
    EMA_F_ENB_GO_DOWN  = 0x00002000, /* 下がる準備が出来た      (狙撃イベントのみ) */
    EMA_F_ENB_WALK     = 0x00004000, /* 歩きに変わる準備が出来た(狙撃イベントのみ) */
    EMA_F_ENB_RUN      = 0x00008000, /* 走りに変わる準備が出来た(狙撃イベントのみ) */
    EMA_F_ENB_RECOVER  = 0x00010000, /* 回復可能                (狙撃イベントのみ) */
    EMA_F_ENB_STUNGRE  = 0x00020000, /* スタングレネード反応可能                   */
    EMA_F_ENB_EYESIGHT = 0x00040000, /* 視線制御                  */
    EMA_F_ENB_IK       = 0x00080000, /* 足IK処理する              */
    EMA_F_ENB_ARM_IK   = 0x00100000, /* 腕IK処理する              */
    EMA_F_ENB_VIBRATE  = 0x00200000, /* 振動可能                  */
    EMA_F_ENB_WANDER   = 0x00400000, /* 徘徊可能                  */

    EMA_F_ATTACKED_BY  = 0x00800000, /* エマは何らかの攻撃を受けた */
    EMA_F_RUN_AWAY     = 0x01000000, /* 逃げる(手繋ぎイベント 舟虫)                */
    EMA_F_FIRST_LINK   = 0x02000000, /* 手繋ぎに向かう  (手繋ぎイベント)*/
    EMA_F_FIRST_WALL   = 0x04000000, /* 壁に向かっている(手繋ぎイベント)*/

    EMA_F_HURT_BY_VMP  = 0x08000000, /* ヴァンプに捕らわれている       */
    EMA_F_HURT_BY_PLY  = 0x10000000, /* プレーヤーからダメージを受けた */
    EMA_F_NEAR_BY_PLY  = 0x20000000, /* プレーヤーから近くを狙われた(狙撃イベントのみ) */

    EMA_F_GAMEOVER     = 0x40000000, /*エマがゲームオーバーにした*/
    EMA_F_EVENT_SNIPE  = 0x80000000, /*狙撃イベント  */
} ;

#endif  /* __EMA_FLAG_H__ */
