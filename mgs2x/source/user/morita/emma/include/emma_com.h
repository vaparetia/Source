/*
  emma_com.h ( emma_msg.c )
  エマ 外部参照用 関数

  2001/03/12 T.Morita
  $Id: emma_com.h,v 1.1.1.3 2002/11/19 11:46:04 Yoshizawa1 Exp $
*/

#ifndef __EMMA_COM_H__
#define __EMMA_COM_H__
/*
  エマの位置(&control.mov)を返します。
  エマが起動していない場合 NULLがきます。
*/
FVECTOR *EMA_CommandGetPosition() ;


/*
  エマのOBJECTへのポインタを返します。
  エマ自体が起動していない場合,NULLを返します。
 */
OBJECT *EMA_CommandGetBody() ;


/*
  エマのCONTROLへのポインタを返します。
  エマ自体が起動していない場合,NULLを返します。
 */
CONTROL *EMA_CommandGetControl() ;


/*
  エマのライフを返します。
  エマ自体が起動していない場合,-1を返します。
 */
int EMA_CommandGetLife() ;


/*
  エマの最大ライフを返します。
  エマ自体が起動していない場合,-1を返します。
 */
int EMA_CommandGetMaxLife() ;


/*
  エマのキャプチャーターゲットへのポインタを返します。
  エマ自体が起動していない場合,NULLを返します。
 */
CAPTURE_TARGET *EMA_CommandGetCaptureTarget() ;


/*
  エマのを攻撃する場合に呼び出して下さい。
  エマがしゃがんで止まります。
  実際には,work->enemy[i]に名前が登録されます。
  成功なら１失敗なら０が返ります。
  （登録は,現在4人まで,つまり同時攻撃が4人までということです）
 */
int EMA_CommandEneAttack( int name ) ;


/*
  攻撃不能状態,または死んだ場合に呼び出して下さい。
  誰も攻撃していなければ,エマが動き出します。
  成功なら１失敗なら０が返ります。

  これを呼ばずに抜けた場合 そのまま動かずにいてしまいますので注意して下さい。
 */
int EMA_CommandEneDied( int name ) ;


/*
  未だ起動していない時は,-1が返ってくる。

  エマにdamage分だけダメージを与える。
  返戻値としてエマのライフとなる.

 */
int EMA_CommandGiveDamage( int damage ) ;


/*
  エマの見つけ用位置(&findpos)を返します。
  XZはコントロールと同じで,Yは体勢によって変わります。

  立1001/座750/寝200      2001.6.6 現在

  エマが起動していない場合 NULLがきます。
*/
FVECTOR *EMA_CommandGetFindPos() ;


/*
  エマが手を繋いでいるかどうか

  EMA_CommandIsEnableLink()
  手を繋げる  :１以上
  手を繋げない:０

  EMA_CommandIsEnableLinkMove()
  動ける  :１以上
  動けない:０

  エマが繋げる状態にあるかどうかを見るための関数
*/
int EMA_CommandIsEnableLink() ;
int EMA_CommandIsEnableLinkMove() ;


/*
  エマが手を繋いでいるかどうか

  手を繋げる  :１以上
  手を繋げない:０

  エマが繋げる状態にあるかどうかを見るための関数
*/
enum emma_state_t
{
    EMA_F_DO_NOTHING,
    EMA_F_I_AM_FAINTED,
    EMA_F_I_AM_SLEEPING,
    EMA_F_I_SAT_DOWN,
    EMA_F_ATTKED_BY_PLAYER,
    EMA_F_ATTKED_BY_OTHERS,
    EMA_F_I_DISLIKE_WORMS,
    EMA_F_I_AM_CAPTURED,
} ;
int EMA_CommandGetStatus() ;


/*
  エマが手を繋いでいるかどうか

  手を繋いでいる  :１以上
  手を繋いでいない:０

  エマが起動していない場合やライデンプラグンが起動していない場合 0 がきます。
*/
int NewEMA_CommandIsLink() ;



/*
  ガンカメが壊れた時に呼び出す

  エマが起動していない場合 0 がきます。そのときカウントされません。
*/
int EMA_GunCameraBroken() ;
int EMA_GetGunCameraBrokenNum() ;

/*

  エマ用の初期化

 */
void EMA_Die() ;
void EMA_Birth( void *ptr ) ;

#endif /*__EMMA_COM_H__*/
