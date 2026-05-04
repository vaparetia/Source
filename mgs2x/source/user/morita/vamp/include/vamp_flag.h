/*
  ema_flag.h
  エマフラグ ヘッダ

  2001/02/08 Y.Korekado
  $Id: vamp_flag.h,v 1.1.1.3 2002/11/19 11:46:36 Yoshizawa1 Exp $
*/

#ifndef __VMPS_FLAG_H__
#define __VMPS_FLAG_H__ 1

/* アクションフラグ */
enum vamp_flag_t
{
    VMPS_F_NONE        = 0x00000000,

    VMPS_F_NON_DAMAGE  = 0x00000001, /* エマは,無敵状態（毎フレームクリアされる） */
    VMPS_F_NON_THINK   = 0x00000002,
    VMPS_F_NON_FACING  = 0x00000004, /* エマは,顔向けしない状態（毎フレームクリアされる） */
    VMPS_F_NON_CAPTURE = 0x00000008, /* エマは,首締め出来ない状態（毎フレームクリアされる） */
    VMPS_F_NON_ACTION  = 0x00000010, /* エマは,動かない状態 */

    VMPS_F_IS_ON_STEP  = 0x00000020,
    VMPS_F_IS_ON_LEFT  = 0x00000100, /* ライデンの左側にいる */
    VMPS_F_IS_SQUAT    = 0x00000200, /*  */
    VMPS_F_IS_ATTACKED = 0x00000400, /* 誰かから攻撃を受けている */

    VMPS_F_GAMEOVER    = 0x20000000, /*エマがゲームオーバーにした*/
    VMPS_F_EVENT_FOLLOW= 0x40000000, /*手繋ぎイベント*/
    VMPS_F_EVENT_SNIPE = 0x80000000, /*狙撃イベント  */
} ;

#endif  /* __VMPS_FLAG_H__ */
