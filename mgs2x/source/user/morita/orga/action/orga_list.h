/*
   orga_lst.h
   オルガ/行動リストヘッダファイル

   1999/12/25 T.Morita
   $Id: orga_list.h,v 1.1.1.3 2002/11/19 11:46:22 Yoshizawa1 Exp $			
*/



/***
  [アクション]
  アクトリストは,アクトによって構成され,アクションと呼ぶ。アクトは,最
  小行動単位で,それ以上粒度の小さいものはない。アクションに登場するア
  クトは,少なくともモーション数以上存在する。つまり,モーションが同じで
  も終了判定や割り込み判定が異なれば,違うアクトとなる。

  アクションは,必ず NULL で終らなければならなく,さらに原則的に最後のア
  クトは,ORG_ActThink でなければならない。なお ORG_ActStillStart を呼
  ぶことで,静止モーションに戻すことができる。

  GM_CheckObject_IsEnd で終了するアクトの後に,GM_ConfigObjectAction の
  含まれるアクトが来ないと,うまく表示されない。


  [アクションリスト]
  アクションリストは,アクションによって構成され,NULLで終わらなければな
  らない。現在のリストの制限は,３２個までである(u_intのマスクを使用し
  ているためで改良可能)。

  アクションリストを一つのカテゴリーとして,行動の思考制御をする。

  ***/



/***

  基本アクションセット

  ***/
void (*ORG_ActionDefault0[])( Work * ) = {
    ORG_ActMotionOnceLayer0, ORG_ActStillStart, ORG_ActThink, NULL
} ;
void (*ORG_ActionDefault1[])( Work * ) = {
    ORG_ActMotionOnceLayer1, ORG_ActStillStart, ORG_ActThink, NULL
} ;
void (*ORG_ActionAbort[])( Work * ) = {
    ORG_ActAbortStart, ORG_ActThink, NULL
} ;


/*リセットアクション*/
void (*ORG_ActionReset[])( Work * ) = {
    ORG_ActResetStart, NULL
} ;

/* 沈黙アクション 現行のアクションを中止し, ORGA_F_STOP_STILLのフラグが解除されるまで沈黙する */
void (*ORG_ActionStopStill[])( Work * ) = {
    ORG_ActStopStillStart, ORG_ActStopStill, ORG_ActThink, NULL
} ;

/* 待ちアクション */
static void (*ORG_ActionThreeSecWait[])( Work * ) = {
    ORG_ActWaitThreeSecStart , ORG_ActWait, ORG_ActThink, NULL
} ;							  
static void (*ORG_ActionTwoSecWait[])( Work * ) = {
    ORG_ActWaitTwoSecStart   , ORG_ActWait, ORG_ActThink, NULL
} ;							  
static void (*ORG_ActionOneSecWait[])( Work * ) = {
    ORG_ActWaitOneSecStart   , ORG_ActWait, ORG_ActThink, NULL
} ;							  
static void (*ORG_ActionHalfSecWait[])( Work * ) = {		  
    ORG_ActWaitHalfSecStart  , ORG_ActWait, ORG_ActThink, NULL
} ;							  
static void (*ORG_ActionQuaterSecWait[])( Work * ) = {		  
    ORG_ActWaitQuaterSecStart, ORG_ActWait, ORG_ActThink, NULL
} ;							  
static void (*ORG_ActionCentiSecWait[])( Work * ) = {		  
    ORG_ActWaitCentiSecStart , ORG_ActWait, ORG_ActThink, NULL
} ;

/* 待ちアクションリスト */
void (**ORG_ActListWait[])( Work * ) = {
    ORG_ActionThreeSecWait,  ORG_ActionTwoSecWait,
    ORG_ActionOneSecWait,    ORG_ActionHalfSecWait, 
    ORG_ActionQuaterSecWait, ORG_ActionCentiSecWait, NULL
} ;



/***

  撃つアクションセット

  ***/


/* 立ちながら 2秒待って撃つ*/
static void (*ORG_ActionStandTwoSecDoubleFire[])( Work * ) = {
    ORG_ActWaitTwoSecStart,ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActStandFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart,ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionStandTwoSecTripleFire[])( Work * ) = {
    ORG_ActWaitTwoSecStart,ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActStandFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart,ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/* 立ちながら 1秒待って撃つ*/
static void (*ORG_ActionStandOneSecDoubleFire[])( Work * ) = {
    ORG_ActWaitOneSecStart,ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActStandFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart,ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionStandOneSecTripleFire[])( Work * ) = {
    ORG_ActWaitOneSecStart,ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActStandFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart,ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/* 立ちながら 0.25秒待って撃つ*/
static void (*ORG_ActionStandQuaterSecDoubleFire[])( Work * ) = {
    ORG_ActWaitQuaterSecStart,ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActStandFireStart,ORG_ActWaitQuaterSecStart,ORG_ActMotionOnceLayer0,
    ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActStandFireStart,ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionStandQuaterSecTripleFire[])( Work * ) = {
    ORG_ActWaitQuaterSecStart,ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActStandFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart,ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/* しゃがんで 2秒待って撃つ*/
static void (*ORG_ActionSquatTwoSecDoubleFire[])( Work * ) = {
    ORG_ActWaitTwoSecStart,ORG_ActSquatFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActSquatFireStart,ORG_ActMotionOnceLayer1,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatTwoSecTripleFire[])( Work * ) = {
    ORG_ActWaitTwoSecStart,ORG_ActSquatFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActSquatFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActSquatFireStart,ORG_ActMotionOnceLayer1,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/* しゃがんで 1秒待って撃つ*/
static void (*ORG_ActionSquatOneSecDoubleFire[])( Work * ) = {
    ORG_ActWaitOneSecStart,ORG_ActSquatFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatFireStart,ORG_ActMotionOnceLayer1,
    ORG_ActSquatFireStart,ORG_ActMotionOnceLayer1,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatOneSecTripleFire[])( Work * ) = {
    ORG_ActWaitOneSecStart,ORG_ActSquatFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActSquatFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActSquatFireStart,ORG_ActMotionOnceLayer1,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/* しゃがんで 0.5秒待って撃つ*/
static void (*ORG_ActionSquatHalfSecDoubleFire[])( Work * ) = {
    ORG_ActWaitHalfSecStart,ORG_ActSquatFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatFireStart,ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActSquatFireStart,ORG_ActMotionOnceLayer1,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatHalfSecTripleFire[])( Work * ) = {
    ORG_ActWaitHalfSecStart,ORG_ActSquatFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActSquatFireStart,ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActSquatFireStart,ORG_ActMotionOnceLayer1,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

static void (**ORG_ActListFire[])( Work * ) = {
    ORG_ActionStandTwoSecDoubleFire  , ORG_ActionStandTwoSecTripleFire ,
    ORG_ActionStandOneSecDoubleFire  , ORG_ActionStandOneSecTripleFire ,
    ORG_ActionStandQuaterSecDoubleFire, ORG_ActionStandQuaterSecTripleFire,

    ORG_ActionSquatTwoSecDoubleFire  , ORG_ActionSquatTwoSecTripleFire ,
    ORG_ActionSquatOneSecDoubleFire  , ORG_ActionSquatOneSecTripleFire ,
    ORG_ActionSquatHalfSecDoubleFire , ORG_ActionSquatHalfSecTripleFire,
} ;


/*

  しゃがみから立って撃つセット

*/
/* しゃがみから素早く立って 0.5秒待って撃つ*/
static void (*ORG_ActionFastStandingHalfSecDoubleFire[])( Work * ) = {
    ORG_ActFastStandingFireStart, ORG_ActMotionOnceLayer0,
    ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActWaitHalfSecStart, ORG_ActFireP,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart, ORG_ActMotionOnceLayer0,
    ORG_ActStandingFireEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionFastStandingHalfSecTripleFire[])( Work * ) = {
    ORG_ActFastStandingFireStart, ORG_ActMotionOnceLayer0,
    ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActWaitHalfSecStart, ORG_ActFireP,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart, ORG_ActMotionOnceLayer0,
    ORG_ActStandingFireEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/* しゃがみから普通に立って 0.5秒待って撃つ*/
static void (*ORG_ActionSlowStandingOneSecDoubleFire[])( Work * ) = {
    ORG_ActSlowStandingFireStart, ORG_ActMotionOnceLayer0,
    ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActWaitOneSecStart, ORG_ActFireP,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart, ORG_ActMotionOnceLayer0,
    ORG_ActStandingFireEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSlowStandingOneSecTripleFire[])( Work * ) = {
    ORG_ActSlowStandingFireStart, ORG_ActMotionOnceLayer0,
    ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActWaitOneSecStart, ORG_ActFireP,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart, ORG_ActMotionOnceLayer0,
    ORG_ActStandingFireEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

static void (**ORG_ActListStandingFire[])( Work * ) = {
    ORG_ActionSlowStandingOneSecDoubleFire, ORG_ActionSlowStandingOneSecTripleFire,
    ORG_ActionSlowStandingOneSecDoubleFire, ORG_ActionSlowStandingOneSecTripleFire,

    ORG_ActionFastStandingHalfSecDoubleFire, ORG_ActionFastStandingHalfSecTripleFire,
    ORG_ActionFastStandingHalfSecDoubleFire, ORG_ActionFastStandingHalfSecTripleFire,
} ;


/*
  マシンガン撃ち
*/
static void (*ORG_ActionMachineGunFellow[])( Work * ) = {
    ORG_ActStandFirePStart, ORG_ActFireAimmingShot, ORG_ActWaitHalfSecStart, ORG_ActFireP,
    ORG_ActStandFireStart,ORG_ActWaitDeciSecStart, ORG_ActBlindFire,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/*
  スネーク狙い撃ち
*/
void (*ORG_ActionSnipeSnake[])( Work * ) = {
    ORG_ActSnipeSnakeMoveStart,
    ORG_ActFaceToTargetStart, ORG_ActWaitFacing, ORG_ActDashStart, ORG_ActRunOrDash, 
    ORG_ActStandFirePStart, ORG_ActWaitZeroSecStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActSnipeFireStart, ORG_ActMotionOnceLayer0,
    ORG_ActStandFirePStart, ORG_ActSnipeFireP,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;



/*
  スネークのエルード撃ち
*/
#if 0
static void (*ORG_ActionErudeKiller[])( Work * ) = 
{
    ORG_ActFastPeepingRightStart, ORG_ActWaitOneSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActErudeFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
#else
static void (*ORG_ActionErudeKiller[])( Work * ) = 
{
    ORG_ActErudeFirePStart, ORG_ActFireAimmingShot, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActErudeFireStart, ORG_ActErudeFire,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
#endif

/*
  リロード
*/
static void (*ORG_ActionStandReload[])( Work * ) = {
    ORG_ActStandReloadStart, ORG_ActReloading, ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatReload[])( Work * ) = {
    ORG_ActSquatReloadStart, ORG_ActReloading, ORG_ActStillStart, ORG_ActThink, NULL
} ;

static void (**ORG_ActListReload[])( Work * ) = {
    ORG_ActionStandReload, ORG_ActionSquatReload,
    ORG_ActionStandReload, ORG_ActionSquatReload,
} ;


/*
  グレネードなげ
*/
static void (*ORG_ActionStandThroughGrenade[])( Work * ) = {
    ORG_ActFaceToPlayerStart, ORG_ActWaitFacing,
    ORG_ActThroughGrenadeStart, ORG_ActThroughGrenade,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionUnrecogThroughGrenade[])( Work * ) = {
    ORG_ActUnrecogThroughGrenadeStart, ORG_ActWaitFacing,
    ORG_ActThroughGrenade,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSnakeHideThroughGrenade[])( Work * ) = {
    ORG_ActFaceToPlayerStart, ORG_ActWaitFacing,
    ORG_ActThroughGrenadeLowStart, ORG_ActThroughGrenadeToGap,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionThroughGrenadeLow[])( Work * ) = {
    ORG_ActFaceToPlayerStart, ORG_ActWaitFacing,
    ORG_ActThroughGrenadeLowStart, ORG_ActThroughGrenadeLow,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  ポテト壊し
  とりあえずなし

void (*ORG_ActionBreakingPotato1[])( Work * ) = {
    ORG_ActFireSequence1Start, ORG_ActWaitFacing, ORG_ActFirePotato1Start, NULL
} ;
void (*ORG_ActionBreakingPotato2[])( Work * ) = {
    ORG_ActFireSequence2Start, ORG_ActWaitFacing, ORG_ActFirePotato2Start, NULL
} ;
 */

/********************************************************

  覗き撃ちアクションセット
  覗きアクションセット

  ********************************************************/

/*                覗きアクションセット                   */

/*
  ビハインド覗き左
*/
static void (*ORG_ActionBehindFastHalfSecPeepingLeft[])( Work * ) = {
    ORG_ActBehindFastPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBehindPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindFastCentiSecPeepingLeft[])( Work * ) = {
    ORG_ActBehindFastPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActBehindPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSlowHalfSecPeepingLeft[])( Work * ) = {
    ORG_ActBehindSlowPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBehindPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSlowCentiSecPeepingLeft[])( Work * ) = {
    ORG_ActBehindSlowPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActBehindPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  座りビハインド覗き左
*/
static void (*ORG_ActionBehindSquatFastHalfSecPeepingLeft[])( Work * ) = {
    ORG_ActBehindSquatFastPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSquatSlowHalfSecPeepingLeft[])( Work * ) = {
    ORG_ActBehindSquatSlowPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSquatFastCentiSecPeepingLeft[])( Work * ) = {
    ORG_ActBehindSquatFastPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSquatSlowCentiSecPeepingLeft[])( Work * ) = {
    ORG_ActBehindSquatSlowPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/*
  立ち覗き左
 */
static void (*ORG_ActionFastHalfSecPeepingLeft[])( Work * ) = {
    ORG_ActFastPeepingNoshootLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionFastCentiSecPeepingLeft[])( Work * ) = {
    ORG_ActFastPeepingNoshootLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSlowHalfSecPeepingLeft[])( Work * ) = {
    ORG_ActSlowPeepingNoshootLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSlowCentiSecPeepingLeft[])( Work * ) = {
    ORG_ActSlowPeepingNoshootLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/*
  しゃがみ覗き左
 */
static void (*ORG_ActionSquatFastHalfSecPeepingLeft[])( Work * ) = {
    ORG_ActSquatFastPeepingNoshootLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatSlowHalfSecPeepingLeft[])( Work * ) = {
    ORG_ActSquatSlowPeepingNoshootLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatFastCentiSecPeepingLeft[])( Work * ) = {
    ORG_ActSquatFastPeepingNoshootLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatSlowCentiSecPeepingLeft[])( Work * ) = {
    ORG_ActSquatSlowPeepingNoshootLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;



/*
  ビハインド覗き右
*/
static void (*ORG_ActionBehindFastHalfSecPeepingRight[])( Work * ) = {
    ORG_ActBehindFastPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBehindPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindFastCentiSecPeepingRight[])( Work * ) = {
    ORG_ActBehindFastPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActBehindPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSlowHalfSecPeepingRight[])( Work * ) = {
    ORG_ActBehindSlowPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBehindPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSlowCentiSecPeepingRight[])( Work * ) = {
    ORG_ActBehindSlowPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActBehindPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  座りビハインド覗き右
*/
static void (*ORG_ActionBehindSquatFastHalfSecPeepingRight[])( Work * ) = {
    ORG_ActBehindSquatFastPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSquatSlowHalfSecPeepingRight[])( Work * ) = {
    ORG_ActBehindSquatSlowPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSquatFastCentiSecPeepingRight[])( Work * ) = {
    ORG_ActBehindSquatFastPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindSquatSlowCentiSecPeepingRight[])( Work * ) = {
    ORG_ActBehindSquatSlowPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  立ち覗き右
 */
static void (*ORG_ActionFastHalfSecPeepingRight[])( Work * ) = {
    ORG_ActFastPeepingNoshootRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSlowHalfSecPeepingRight[])( Work * ) = {
    ORG_ActSlowPeepingNoshootRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionFastCentiSecPeepingRight[])( Work * ) = {
    ORG_ActFastPeepingNoshootRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSlowCentiSecPeepingRight[])( Work * ) = {
    ORG_ActSlowPeepingNoshootRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/*
  しゃがみ覗き右
 */
static void (*ORG_ActionSquatFastHalfSecPeepingRight[])( Work * ) = {
    ORG_ActSquatFastPeepingNoshootRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatSlowHalfSecPeepingRight[])( Work * ) = {
    ORG_ActSquatSlowPeepingNoshootRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingPStart, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatFastCentiSecPeepingRight[])( Work * ) = {
    ORG_ActSquatFastPeepingNoshootRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatSlowCentiSecPeepingRight[])( Work * ) = {
    ORG_ActSquatSlowPeepingNoshootRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingPStart, ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/*
  中腰覗き
 */
#if 0
static void (*ORG_ActionHalfStandPeeping[])( Work * ) = {
    ORG_ActHalfStandPeepingStart, ORG_ActHalfStandPeeping,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
#else
static void (*ORG_ActionHalfStandPeeping[])( Work * ) = {
    ORG_ActHalfStandPeepingSlowStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActStandFirePStart, ORG_ActFireAimmingShot,  ORG_ActWaitCentiSecStart, ORG_ActWait,
    ORG_ActHalfStandPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
#endif

static void (**ORG_ActListModistPeepingLeft[])( Work * ) =
{
    ORG_ActionSlowCentiSecPeepingLeft,
    ORG_ActionSquatSlowCentiSecPeepingLeft,
    ORG_ActionBehindSlowCentiSecPeepingLeft,
    ORG_ActionBehindSquatSlowCentiSecPeepingLeft,

    ORG_ActionFastCentiSecPeepingLeft,
    ORG_ActionSquatFastCentiSecPeepingLeft,
    ORG_ActionBehindFastCentiSecPeepingLeft,
    ORG_ActionBehindSquatFastCentiSecPeepingLeft,

    ORG_ActionSlowHalfSecPeepingLeft,
    ORG_ActionSquatSlowHalfSecPeepingLeft,
    ORG_ActionBehindSlowHalfSecPeepingLeft,
    ORG_ActionBehindSquatSlowHalfSecPeepingLeft,

    ORG_ActionFastHalfSecPeepingLeft,
    ORG_ActionSquatFastHalfSecPeepingLeft,
    ORG_ActionBehindFastHalfSecPeepingLeft,
    ORG_ActionBehindSquatFastHalfSecPeepingLeft,
} ;

static void (**ORG_ActListModistPeepingRight[])( Work * ) =
{
    ORG_ActionSlowCentiSecPeepingRight,
    ORG_ActionSquatSlowCentiSecPeepingRight,
    ORG_ActionBehindSlowCentiSecPeepingRight,
    ORG_ActionBehindSquatSlowCentiSecPeepingRight,

    ORG_ActionFastCentiSecPeepingRight,
    ORG_ActionSquatFastCentiSecPeepingRight,
    ORG_ActionBehindFastCentiSecPeepingRight,
    ORG_ActionBehindSquatFastCentiSecPeepingRight,

    ORG_ActionSlowHalfSecPeepingRight,
    ORG_ActionSquatSlowHalfSecPeepingRight,
    ORG_ActionBehindSlowHalfSecPeepingRight,
    ORG_ActionBehindSquatSlowHalfSecPeepingRight,

    ORG_ActionFastHalfSecPeepingRight,
    ORG_ActionSquatFastHalfSecPeepingRight,
    ORG_ActionBehindFastHalfSecPeepingRight,
    ORG_ActionBehindSquatFastHalfSecPeepingRight,
} ;



/*              覗き撃ちアクションセット                 */

/*
  狙われても避けずに
  立ち覗き2秒待って攻撃右（EASY）
 */
static void (*ORG_ActionNoFearSlowPeepingFireRight[])( Work * ) = {
    ORG_ActSlowPeepingRightStart, ORG_ActWaitTwoSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart,  ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  立ち覗き1秒待って攻撃右(NORM)
 */
static void (*ORG_ActionSlowOneSecPeepingDoubleFireRight[])( Work * ) = {
    ORG_ActSlowPeepingRightStart, ORG_ActWaitOneSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSlowOneSecPeepingTripleFireRight[])( Work * ) = {
    ORG_ActSlowPeepingRightStart, ORG_ActWaitOneSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  立ち覗き0.5秒待って攻撃右(HARD)
 */
static void (*ORG_ActionSlowHalfSecPeepingTripleFireRight[])( Work * ) = {
    ORG_ActSlowPeepingRightStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSlowHalfSecPeepingQuadpleFireRight[])( Work * ) = {
    ORG_ActSlowPeepingRightStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  立ち覗き待たずに攻撃右(MANIA)
 */
static void (*ORG_ActionFastZeroSecPeepingTripleFireRight[])( Work * ) = {
    ORG_ActFastPeepingRightStart, ORG_ActWaitZeroSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionFastZeroSecPeepingQuadpleFireRight[])( Work * ) = {
    ORG_ActFastPeepingRightStart, ORG_ActWaitZeroSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;





/*
  狙われても避けずに
  しゃがみ覗き2秒待って攻撃右（EASY）
 */
static void (*ORG_ActionSquatNoFearSlowPeepingFireRight[])( Work * ) = {
    ORG_ActSquatSlowPeepingRightStart, ORG_ActWaitTwoSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart,  ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  しゃがみ覗き1秒待って攻撃右(NORM)
 */
static void (*ORG_ActionSquatSlowOneSecPeepingDoubleFireRight[])( Work * ) = {
    ORG_ActSquatSlowPeepingRightStart, ORG_ActWaitOneSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatSlowOneSecPeepingTripleFireRight[])( Work * ) = {
    ORG_ActSquatSlowPeepingRightStart, ORG_ActWaitOneSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  しゃがみ覗き0.5秒待って攻撃右(HARD)
 */
static void (*ORG_ActionSquatSlowHalfSecPeepingTripleFireRight[])( Work * ) = {
    ORG_ActSquatSlowPeepingRightStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatSlowHalfSecPeepingQuadpleFireRight[])( Work * ) = {
    ORG_ActSquatSlowPeepingRightStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/*
  しゃがみ覗き0.5秒待って攻撃右(MANIA)
 */
static void (*ORG_ActionSquatFastZeroSecPeepingTripleFireRight[])( Work * ) = {
    ORG_ActSquatFastPeepingRightStart, ORG_ActWaitZeroSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatFastZeroSecPeepingQuadpleFireRight[])( Work * ) = {
    ORG_ActSquatFastPeepingRightStart, ORG_ActWaitZeroSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/*覗き込み撃ち右 アクションリスト */

static void (**ORG_ActListPeepingFireRight[])( Work * ) = {
    ORG_ActionNoFearSlowPeepingFireRight,
    ORG_ActionNoFearSlowPeepingFireRight,
    ORG_ActionSlowOneSecPeepingDoubleFireRight ,
    ORG_ActionSlowOneSecPeepingTripleFireRight ,
    ORG_ActionSlowHalfSecPeepingTripleFireRight,
    ORG_ActionSlowHalfSecPeepingQuadpleFireRight,
    ORG_ActionFastZeroSecPeepingTripleFireRight,
    ORG_ActionFastZeroSecPeepingQuadpleFireRight,

    ORG_ActionSquatNoFearSlowPeepingFireRight,
    ORG_ActionSquatNoFearSlowPeepingFireRight,
    ORG_ActionSquatSlowOneSecPeepingDoubleFireRight ,
    ORG_ActionSquatSlowOneSecPeepingTripleFireRight ,
    ORG_ActionSquatSlowHalfSecPeepingTripleFireRight,
    ORG_ActionSquatSlowHalfSecPeepingQuadpleFireRight,
    ORG_ActionSquatFastZeroSecPeepingTripleFireRight,
    ORG_ActionSquatFastZeroSecPeepingQuadpleFireRight,
} ;







/*
  狙われても避けずに
  しゃがみ覗き2秒待って攻撃左（EASY）
 */
static void (*ORG_ActionNoFearSlowPeepingFireLeft[])( Work * ) = {
    ORG_ActSlowPeepingLeftStart, ORG_ActWaitTwoSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  立ち覗き1秒待って攻撃左(NORM)
 */
static void (*ORG_ActionSlowOneSecPeepingDoubleFireLeft[])( Work * ) = {
    ORG_ActSlowPeepingLeftStart, ORG_ActWaitOneSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSlowOneSecPeepingTripleFireLeft[])( Work * ) = {
    ORG_ActSlowPeepingLeftStart, ORG_ActWaitOneSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
/*
  立ち覗き0.5秒待って攻撃左(HARD)
 */
static void (*ORG_ActionSlowHalfSecPeepingTripleFireLeft[])( Work * ) = {
    ORG_ActSlowPeepingLeftStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSlowHalfSecPeepingQuadpleFireLeft[])( Work * ) = {
    ORG_ActSlowPeepingLeftStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  立ち覗き待たずに攻撃左(MANIA)
 */
void (*ORG_ActionFastZeroSecPeepingTripleFireLeft[])( Work * ) = {
    ORG_ActFastPeepingLeftStart, ORG_ActWaitZeroSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionFastZeroSecPeepingQuadpleFireLeft[])( Work * ) = {
    ORG_ActFastPeepingLeftStart, ORG_ActWaitZeroSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;






/*
  狙われても避けずに
  しゃがみ覗き2秒待って攻撃左（EASY）
 */
static void (*ORG_ActionSquatNoFearSlowPeepingFireLeft[])( Work * ) = {
    ORG_ActSquatSlowPeepingLeftStart, ORG_ActWaitTwoSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  しゃがみ覗き1秒待って攻撃左(NORM)
 */
static void (*ORG_ActionSquatSlowOneSecPeepingDoubleFireLeft[])( Work * ) = {
    ORG_ActSquatSlowPeepingLeftStart, ORG_ActWaitOneSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatSlowOneSecPeepingTripleFireLeft[])( Work * ) = {
    ORG_ActSquatSlowPeepingLeftStart, ORG_ActWaitOneSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/*
  しゃがみ覗き0.25秒待って攻撃左(HARD)
 */
static void (*ORG_ActionSquatSlowHalfSecPeepingTripleFireLeft[])( Work * ) = {
    ORG_ActSquatSlowPeepingLeftStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatSlowHalfSecPeepingQuadpleFireLeft[])( Work * ) = {
    ORG_ActSquatSlowPeepingLeftStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  しゃがみ覗き待たずに攻撃左(MANIA)
 */
void (*ORG_ActionSquatFastZeroSecPeepingTripleFireLeft[])( Work * ) = {
    ORG_ActSquatFastPeepingLeftStart, ORG_ActWaitZeroSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatFastZeroSecPeepingQuadpleFireLeft[])( Work * ) = {
    ORG_ActSquatFastPeepingLeftStart, ORG_ActWaitZeroSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;



static void (**ORG_ActListPeepingFireLeft[])( Work * ) = {
    ORG_ActionNoFearSlowPeepingFireLeft,
    ORG_ActionNoFearSlowPeepingFireLeft,
    ORG_ActionSlowOneSecPeepingDoubleFireLeft ,
    ORG_ActionSlowOneSecPeepingTripleFireLeft ,
    ORG_ActionSlowHalfSecPeepingTripleFireLeft,
    ORG_ActionSlowHalfSecPeepingQuadpleFireLeft,
    ORG_ActionFastZeroSecPeepingTripleFireLeft,
    ORG_ActionFastZeroSecPeepingQuadpleFireLeft,

    ORG_ActionSquatNoFearSlowPeepingFireLeft,
    ORG_ActionSquatNoFearSlowPeepingFireLeft,
    ORG_ActionSquatSlowOneSecPeepingDoubleFireLeft ,
    ORG_ActionSquatSlowOneSecPeepingTripleFireLeft ,
    ORG_ActionSquatSlowHalfSecPeepingTripleFireLeft,
    ORG_ActionSquatSlowHalfSecPeepingQuadpleFireLeft,
    ORG_ActionSquatFastZeroSecPeepingTripleFireLeft,
    ORG_ActionSquatFastZeroSecPeepingQuadpleFireLeft,
} ;



/*

  避けずに確実に撃つ

*/
static void (*ORG_ActionNoFearPeepingFireLeft[])( Work * ) = {
    ORG_ActFastPeepingLeftStart, ORG_ActWaitZeroSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionNoFearPeepingFireRight[])( Work * ) = {
    ORG_ActFastPeepingRightStart, ORG_ActWaitZeroSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart,  ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatNoFearPeepingFireLeft[])( Work * ) = {
    ORG_ActSquatFastPeepingLeftStart, ORG_ActWaitZeroSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatNoFearPeepingFireRight[])( Work * ) = {
    ORG_ActSquatFastPeepingRightStart, ORG_ActWaitZeroSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart,  ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;



/*

  避けずにクリーンショット

*/
static void (*ORG_ActionCleanShotPeepingLeft[])( Work * ) = {
    ORG_ActFastPeepingLeftStart, ORG_ActWaitZeroSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionCleanShotPeepingRight[])( Work * ) = {
    ORG_ActFastPeepingRightStart, ORG_ActWaitZeroSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart,  ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatCleanShotPeepingLeft[])( Work * ) = {
    ORG_ActSquatFastPeepingLeftStart, ORG_ActWaitZeroSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatCleanShotPeepingRight[])( Work * ) = {
    ORG_ActSquatFastPeepingRightStart, ORG_ActWaitZeroSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActWait,
    ORG_ActPeepingFireStart,  ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActPeepingFireStart, ORG_ActPeepingFire,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;







/*

  覗き込み撃ち込み バージョン

*/
static void (*ORG_ActionLongTermPeepingFireRight[])( Work * ) = {
    ORG_ActSlowPeepingRightStart, ORG_ActWaitHalfSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot,  ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActBlindFire,

    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

static void (*ORG_ActionLongTermPeepingFireLeft[])( Work * ) = {
    ORG_ActSlowPeepingLeftStart, ORG_ActWaitHalfSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActPeepingFirePStart, ORG_ActFireAimmingShot,  ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActBlindFire,

    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

static void (*ORG_ActionLongTermSquatPeepingFireRight[])( Work * ) = {
    ORG_ActSquatSlowPeepingRightStart, ORG_ActWaitHalfSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActBlindFire,

    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

static void (*ORG_ActionLongTermSquatPeepingFireLeft[])( Work * ) = {
    ORG_ActSquatSlowPeepingLeftStart, ORG_ActWaitHalfSecStart, ORG_ActMotionOnceLayer0,
    ORG_ActSquatPeepingFirePStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActPeepingFireStart, ORG_ActWaitDeciSecStart, ORG_ActBlindFire,

    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;






/*

  ビハインドから飛びだし撃ち

*/
static void (*ORG_ActionBehindPeepingFireLeft[])( Work * ) = {
    //ORG_ActBehindFastPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingFireLeftStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindStandFirePLeftStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActBehindPeepingFireELeftStart, ORG_ActAvoidMotionOnceLayer0,
    //ORG_ActBehindPeepingFireELeft,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBehindPeepingFireRight[])( Work * ) = {
    //ORG_ActBehindFastPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindPeepingFireRightStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindStandFirePRightStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActStandFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActBehindPeepingFireERightStart, ORG_ActAvoidMotionOnceLayer0,
    //ORG_ActBehindPeepingFireERight,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

static void (*ORG_ActionSquatBehindPeepingFireLeft[])( Work * ) = {
    //ORG_ActBehindSquatFastPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingFireLeftStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatFirePLeftStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActSquatFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingFireELeftStart, ORG_ActAvoidMotionOnceLayer0,
    //ORG_ActBehindPeepingFireELeft,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionSquatBehindPeepingFireRight[])( Work * ) = {
    //ORG_ActBehindSquatFastPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatPeepingFireRightStart, ORG_ActWaitHalfSecStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActBehindSquatFirePRightStart, ORG_ActFireAimmingShot, ORG_ActFireP,
    ORG_ActSquatFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActSquatFireStart, ORG_ActWaitDeciSecStart, ORG_ActWait,
    ORG_ActBehindSquatPeepingFireERightStart, ORG_ActAvoidMotionOnceLayer0,
    //ORG_ActBehindPeepingFireERight,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*

  めくら撃ち（左右上）

*/
static void (*ORG_ActionBlindFireLeft[])( Work * ) = {
    ORG_ActBlindFirePLeftStart, ORG_ActFireAimmingShot, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBlindFireRightLeftStart, ORG_ActWaitDeciSecStart, ORG_ActBlindFire,
    ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBlindFireRight[])( Work * ) = {
    ORG_ActBlindFirePRightStart, ORG_ActFireAimmingShot, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBlindFireRightLeftStart,ORG_ActWaitDeciSecStart, ORG_ActBlindFire,
    ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionBlindFireUp[])( Work * ) = {
    ORG_ActBlindFirePUpStart, ORG_ActFireAimmingShot, ORG_ActWaitHalfSecStart, ORG_ActWait,
    ORG_ActBlindFireUpStart, ORG_ActWaitDeciSecStart, ORG_ActBlindFire,
    ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;





/***


  ***/



/***

  移動アクションセット

  ***/
/*かがみ走り*/
static void (*ORG_ActionCrowRun[])( Work * ) = {
    ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
    ORG_ActCrowRunStart, ORG_ActRunOrDash, ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*走り→飛び込み前転*/    
static void (*ORG_ActionRunTumble[])( Work * ) = {
    ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
    ORG_ActRunStart, ORG_ActRunOrDashToTumble, ORG_ActTumbleStart, ORG_ActTumble,
    ORG_ActRunStart, ORG_ActRunOrDash,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
/* 飛び込み前転→ダッシュ   とりあえずなしです*/
static void (*ORG_ActionTumbleRun[])( Work * ) = {
    ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
    ORG_ActTumbleStart, ORG_ActTumble, ORG_ActRunStart, ORG_ActRunOrDash,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
/* ダッシュ→飛び込み前転 */
static void (*ORG_ActionDashTumble[])( Work * ) = {
    ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
    ORG_ActDashStart, ORG_ActRunOrDashToTumble, ORG_ActTumbleStart, ORG_ActTumble,
    ORG_ActDashStart, ORG_ActRunOrDash,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
/* 飛び込み前転→ダッシュ  これも   とりあえずなしです*/
static void (*ORG_ActionTumbleDash[])( Work * ) = {
    ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
    ORG_ActTumbleStart, ORG_ActTumble, ORG_ActDashStart, ORG_ActRunOrDash,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
/*走り→スライディング*/
static void (*ORG_ActionRunSliding[])( Work * ) = {
    ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
    ORG_ActRunStart, ORG_ActRunOrDashToSliding, ORG_ActSlidingStart, ORG_ActSliding,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;



/*走る*/
static void (*ORG_ActionFaceAndRun[])( Work * ) = {
    ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
    ORG_ActRunStart, ORG_ActRunOrDash, ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionFaceAndDash[])( Work * ) = {
    ORG_ActFaceToTargetStart, ORG_ActWaitFacing,
    ORG_ActDashStart, ORG_ActRunOrDash, ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*転がり移動*/
static void (*ORG_ActionRolloutRun[])( Work * ) = {
    ORG_ActFaceToWallStart, ORG_ActWaitFacing,
    ORG_ActRollStart, ORG_ActMotionOnceLayer0, ORG_ActSquatStillStart, 
    ORG_ActSquatRollFirePStart, ORG_ActFireAimmingShot, ORG_ActWaitCentiSecStart, ORG_ActRollFireP,
    ORG_ActSquatRollFireStart, ORG_ActRollMotionOnce,
    ORG_ActSquatRollFireStart, ORG_ActRollMotionOnce,
    ORG_ActRunStart, ORG_ActRunOrDash,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/* 移動撃ち */
static void (*ORG_ActionAimmingDash[])( Work * ) = {
    ORG_ActAimmingDashStart, ORG_ActAimmingDash, ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionAimmingDashWithRage[])( Work * ) = {
    ORG_ActAimmingDashStart, ORG_ActAimmingDashWithRage, ORG_ActStillStart, ORG_ActThink, NULL
} ;

/* ビハインド移動 */
static void (*ORG_ActionBehindWalk[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActBehindWalkStart, ORG_ActBehindWalk, ORG_ActStillStart, ORG_ActThink, NULL
} ;






/***

  その他特殊アクションセット

  ***/

/*
  ホロ開け撃ち
*/
static void (*ORG_ActionOpenHolo[])( Work * ) = 
{
    ORG_ActAimToCutHoloStart, ORG_ActFireP,
    ORG_ActHoloFireStart,

    ORG_ActAimToCutNextStart, ORG_ActFireP,
    ORG_ActHoloFireStart, ORG_ActMotionOnceLayer0,

    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/*
  ホロ飛ばし撃ち（前側）
*/
static void (*ORG_ActionBlowHolo[])( Work * ) = 
{
    ORG_ActBlowOffHoloStart, ORG_ActFireP,
    ORG_ActBlowOffFireStart, ORG_ActBlowOffFire,
    ORG_ActStillStart, ORG_ActThink, NULL

} ;

/*
  投光器撃ち
*/
static void (*ORG_ActionBreakSpotLight[])( Work * ) = {
    ORG_ActAimToSpotLightStart, ORG_ActFireP,
    ORG_ActFireToSpotLightStart, ORG_ActFireToSpotLight,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/* 移動ホロ綱撃ち */
static void (*ORG_ActionAimmingHoloDash[])( Work * ) = {
    ORG_ActAimmingHoloDashStart, ORG_ActAimmingDash, ORG_ActStillStart, ORG_ActThink, NULL
} ;


/* 迷い構え */
static void (*ORG_ActionUnrecog[])( Work * ) = {
    ORG_ActUnrecogPeepingStandStart, 
    ORG_ActUnrecogFireP, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionUnrecogPeepLeft[])( Work * ) = {
    ORG_ActSlowPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActUnrecogPeepingStandStart, ORG_ActUnrecogFireP,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionUnrecogPeepRight[])( Work * ) = {
    ORG_ActSlowPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActUnrecogPeepingStandStart, ORG_ActUnrecogFireP,
    ORG_ActPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionUnrecogSquatPeepLeft[])( Work * ) = {
    ORG_ActSquatSlowPeepingLeftStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActUnrecogPeepingSquatStart, ORG_ActUnrecogFireP,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
static void (*ORG_ActionUnrecogSquatPeepRight[])( Work * ) = {
    ORG_ActSquatSlowPeepingRightStart, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActUnrecogPeepingSquatStart, ORG_ActUnrecogFireP,
    ORG_ActSquatPeepingEStart, ORG_ActMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;


/* 冷やかし */
static void (*ORG_ActionTeasing1[])( Work * ) = {
    ORG_ActTease1Start, ORG_ActTease1,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

static void (*ORG_ActionTeasing2[])( Work * ) = {
    ORG_ActTease2Start, ORG_ActAvoidMotionOnceLayer0,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;

/* 「まだよ」「やるわね」を言うときのカメラカット */
static void (*ORG_ActionTeaseSpeakFastLeft[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActTeaseSpeakFastLeftStart,   ORG_ActTeaseSpeakFast,
    ORG_ActTeaseStillFast, ORG_ActThink, NULL
} ;
static void (*ORG_ActionTeaseSpeakFastRight[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActTeaseSpeakFastRightStart,  ORG_ActTeaseSpeakFast,
    ORG_ActTeaseStillFast, ORG_ActThink, NULL
} ;

static void (*ORG_ActionTeaseSpeakFastSquatLeft[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActTeaseSpeakFastSquatLeftStart,   ORG_ActTeaseSpeakFast,
    ORG_ActTeaseStillFast, ORG_ActThink, NULL
} ;
static void (*ORG_ActionTeaseSpeakFastSquatRight[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActTeaseSpeakFastSquatRightStart,  ORG_ActTeaseSpeakFast,
    ORG_ActTeaseStillFast, ORG_ActThink, NULL
} ;

static void (*ORG_ActionTeaseSpeakSlowLeft[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActTeaseSpeakSlowLeftStart,   ORG_ActTeaseSpeakSlow,
    ORG_ActTeaseStillSlow, ORG_ActThink, NULL
} ;
static void (*ORG_ActionTeaseSpeakSlowRight[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActTeaseSpeakSlowRightStart,  ORG_ActTeaseSpeakSlow,
    ORG_ActTeaseStillSlow, ORG_ActThink, NULL
} ;

static void (*ORG_ActionTeaseSpeakSlowSquatLeft[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActTeaseSpeakSlowSquatLeftStart,   ORG_ActTeaseSpeakSlow,
    ORG_ActTeaseStillSlow, ORG_ActThink, NULL
} ;
static void (*ORG_ActionTeaseSpeakSlowSquatRight[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActTeaseSpeakSlowSquatRightStart,  ORG_ActTeaseSpeakSlow,
    ORG_ActTeaseStillSlow, ORG_ActThink, NULL
} ;

/* 人生語りカットイン用アクション 最初 */
static void (*ORG_ActionTeaseSpeakStreamFirst[])( Work * ) = {
    ORG_ActStandFirePStart, ORG_ActTeaseSpeakStream,
    ORG_ActWaitTwoSecStart, ORG_ActTeaseStillStreamWait,
    ORG_ActStandReloadStart, ORG_ActReloading,
    
    ORG_ActStandStillStart,
    ORG_ActTeaseStillStreamFirst, ORG_ActThink, NULL
} ;
/* 人生語りカットイン用アクション 最後 */
static void (*ORG_ActionTeaseSpeakStreamLast[])( Work * ) = {
    ORG_ActBehindToWallStart, ORG_ActWaitFacing,
    ORG_ActTeaseSpeakSlowLeftStart,  ORG_ActTeaseSpeakStreamLast,
    ORG_ActTeaseStillStreamLast, ORG_ActThink, NULL
} ;


/* まず, マガジンを見て 白？マーク それから, 迷い構えを行なう */
static void (*ORG_ActionMagazineThrown[])( Work * ) = {

    ORG_ActWaitHalfSecStart, ORG_ActStandFirePStart, ORG_ActFireP,
    ORG_ActOnceMoreMagazineStart, ORG_ActFireP, ORG_ActEndOfMagazineStart,
    ORG_ActStillStart, ORG_ActThink, NULL
} ;
