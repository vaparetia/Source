/*
   brk_flr_def.h
   揺れ落下 シグナルdefine

   1999/12/13 T. Morita
   $Id: brk_flr_def.h,v 1.1.1.3 2002/11/19 11:45:47 Yoshizawa1 Exp $
*/

/*
  崩落床のモデルの移動やエフェクトを出します。このキャラは,基本的に親
  子関係を使い,シグナルを使って制御します。GV_ACT_EXのキャラで以下のよ
  うに親子関係を付けて起動します。

  GV_SetActorChild( work, NewFallingFloor( model, &work->mov, &work->rot ) ) ;

  その後,アクションを起こしたい時に

  GV_CallChildSignalFunc( work, BRK_FLR_ModeSwingStart, 0 ) ;

  を呼ぶことでアクションが実行されます。シグナル関数で渡すvalue引数は,
  各アクションごとに異なりますので,以下のものを参考にして下さい。

      BRK_FLR_ModeNone : 制御が止まります。一時停止状態になります。表
      示は消えません。
          value引数は無し

      BRK_FLR_ModeReset : 表示を元の位置,回転状態に戻します。
          value引数は無し

      BRK_FLR_ModeShakeStart : 揺れが開始します。（揺れの初期化を含む）
          value引数は落ちるまでのフレーム数
      BRK_FLR_ModeShake : 使用禁止
          value引数は落ちるまでのフレーム数

      BRK_FLR_ModeSwingStart : 落下前の振り子が開始します。（2回振れる
      と崩落を開始します）
          value引数は振り子になる場所を指定します。モデルの相対位置的に
          0 :左前端 以下1点がついた状態のように
          1 :右前端
          2 :左後端
          3 :右後端
          4 :左端   以下蝶番のように
          5 :前端
          6 :後端
          7 :右端
      BRK_FLR_ModeSwing : 使用禁止
          value引数は無し

      BRK_FLR_ModeFallStart : 崩落を開始します。キャラの初期化時にハザー
      ドをチェックしており,そこまで落下し,自動的に倒れに入ります。
          value引数は０の場合,ランダンム角速度が初期値に入ります。
      BRK_FLR_ModeFall : 使用禁止
          value引数は無し

      BRK_FLR_ModeLaydownStart : その場で倒れます。
          value引数は無し
      BRK_FLR_ModeLaydown : 使用禁止
          value引数は無し

      BRK_FLR_SetSeMode: SEを別の音にする
          value引数は ０ と１ で区別する。
	  このシグナルは,起動後すぐに与えること
      BRK_FLR_SetFallType: ShakeStart開始の時に振り子のタイプを設定する
          value引数は BRK_FLR_ModeSwingStart の value と同じ値です。
	  このシグナルは,起動後すぐに与えること
*/

extern void *NewFallingFloor( int model, int thermal,
			      FVECTOR *pos, SVECTOR *rot ) ;


/* 順序変更禁止！！！ */
enum
{
    BRK_FLR_ModeNone = 0,
    BRK_FLR_ModeReset,
    BRK_FLR_ModeShakeStart,
    BRK_FLR_ModeShake,
    BRK_FLR_ModeSwingStart,
    BRK_FLR_ModeSwing,
    BRK_FLR_ModeFallStart,
    BRK_FLR_ModeFall,
    BRK_FLR_ModeLaydownStart,
    BRK_FLR_ModeLaydown,

    BRK_FLR_SetSeMode,
    BRK_FLR_SetFallType,
} ;
