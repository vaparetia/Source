/*
   fort_lst.h
   フォーチュン/行動リストヘッダファイル

   1999/12/25 T.Morita
   $Id: fort_list.h,v 1.1.1.3 2002/11/19 11:46:08 Yoshizawa1 Exp $			
*/



/***
  [アクション]
  アクトリストは,アクトによって構成され,アクションと呼ぶ。アクトは,最
  小行動単位で,それ以上粒度の小さいものはない。アクションに登場するア
  クトは,少なくともモーション数以上存在する。つまり,モーションが同じで
  も終了判定や割り込み判定が異なれば,違うアクトとなる。

  アクションは,必ず NULL で終らなければならなく,さらに原則的に最後のア
  クトは,FRT_ActThink でなければならない。なお FRT_ActStillStart を呼
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
void (*FRT_ActionAbort[])( Work * ) = {
    FRT_ActAbortStart, FRT_ActThink, NULL
} ;

/*リセットアクション*/
void (*FRT_ActionReset[])( Work * ) = {
    FRT_ActResetStart, NULL
} ;

/* 沈黙アクション 現行のアクションを中止し, FRT_F_STOP_STILLのフラグが解除されるまで沈黙する */
void (*FRT_ActionStopStill[])( Work * ) = {
    FRT_ActStopStillStart, FRT_ActStopStill, FRT_ActThink, NULL
} ;

/* 待ちアクション */
static void (*FRT_ActionThreeSecWait[])( Work * ) = {
    FRT_ActWaitThreeSecStart , FRT_ActWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionTwoSecWait[])( Work * ) = {
    FRT_ActWaitTwoSecStart   , FRT_ActWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionOneSecWait[])( Work * ) = {
    FRT_ActWaitOneSecStart   , FRT_ActWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionHalfSecWait[])( Work * ) = {		  
    FRT_ActWaitHalfSecStart  , FRT_ActWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionQuaterSecWait[])( Work * ) = {		  
    FRT_ActWaitQuaterSecStart, FRT_ActWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionCentiSecWait[])( Work * ) = {		  
    FRT_ActWaitCentiSecStart , FRT_ActWait, FRT_ActThink, NULL
} ;

static void (*FRT_ActionThreeSecUnrecogWait[])( Work * ) = {
    FRT_ActWaitThreeSecStart , FRT_ActUnrecogWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionTwoSecUnrecogWait[])( Work * ) = {
    FRT_ActWaitTwoSecStart   , FRT_ActUnrecogWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionOneSecUnrecogWait[])( Work * ) = {
    FRT_ActWaitOneSecStart   , FRT_ActUnrecogWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionHalfSecUnrecogWait[])( Work * ) = {		  
    FRT_ActWaitHalfSecStart  , FRT_ActUnrecogWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionQuaterSecUnrecogWait[])( Work * ) = {		  
    FRT_ActWaitQuaterSecStart, FRT_ActUnrecogWait, FRT_ActThink, NULL
} ;							  
static void (*FRT_ActionCentiSecUnrecogWait[])( Work * ) = {		  
    FRT_ActWaitCentiSecStart , FRT_ActUnrecogWait, FRT_ActThink, NULL
} ;

/* 待ちアクションリスト */
void (**FRT_ActListWait[])( Work * ) = {
    FRT_ActionThreeSecWait,  FRT_ActionTwoSecWait,
    FRT_ActionOneSecWait,    FRT_ActionHalfSecWait, 
    FRT_ActionQuaterSecWait, FRT_ActionCentiSecWait, NULL
} ;


/*デモ中立ち*/
static void (*FRT_ActionStandWhileDemo[])( Work * ) = {
    FRT_ActWaitWhileDemo, FRT_ActStillStart, FRT_ActThink, NULL
} ;


/***

  撃つアクションセット

  ***/
/* 立ちながら 3秒待って撃つ */
void (*FRT_ActionStandThreeSecFire[])( Work * ) = {
    FRT_ActWaitThreeSecStart,
    FRT_ActStandFirePStart, FRT_ActFireP,
    FRT_ActStandFireStart,FRT_ActFire,
    FRT_ActStillStart, FRT_ActThink, NULL
} ;

/* 立ちながら 2秒待って撃つ */
void (*FRT_ActionStandTwoSecFire[])( Work * ) = {
    FRT_ActWaitTwoSecStart,
    FRT_ActStandFirePStart, FRT_ActFireP,
    FRT_ActStandFireStart,FRT_ActFire,
    FRT_ActStillStart, FRT_ActThink, NULL
} ;

/* 立ちながら 1秒待って撃つ */
void (*FRT_ActionStandOneSecFire[])( Work * ) = {
    FRT_ActWaitOneSecStart,
    FRT_ActStandFirePStart, FRT_ActFireP,
    FRT_ActStandFireStart,FRT_ActFire,
    FRT_ActStillStart, FRT_ActThink, NULL
} ;

/* 立ちながら 0.5秒待って撃つ */
static void (*FRT_ActionStandHalfSecFire[])( Work * ) = {
    FRT_ActWaitHalfSecStart,FRT_ActStandFirePStart, FRT_ActFireP,
    FRT_ActStandFireStart,FRT_ActFire,
    FRT_ActStillStart, FRT_ActThink, NULL
} ;

/* 立ちながら 0秒待って撃つ */
static void (*FRT_ActionStandZeroSecFire[])( Work * ) = {
    FRT_ActWaitZeroSecStart,FRT_ActStandFirePStart, FRT_ActFireP,
    FRT_ActStandFireStart,FRT_ActFire,
    FRT_ActStillStart, FRT_ActThink, NULL
} ;

/* 立ちながら 2連射撃つ */
static void (*FRT_ActionStandDoubleFire[])( Work * ) = {
    FRT_ActWaitZeroSecStart,FRT_ActStandFirePStart, FRT_ActFireP,
    FRT_ActWaitQuaterSecStart,FRT_ActStandFireStart,  FRT_ActFireP,
    FRT_ActStandFireStart,FRT_ActFire,
    FRT_ActStillStart, FRT_ActThink, NULL
} ;

/* リロード */
static void (*FRT_ActionStandReload[])( Work * ) = {
    FRT_ActStandReloadStart, FRT_ActReloading, FRT_ActStillStart, FRT_ActThink, NULL
} ;

/* 構える */
static void (*FRT_ActionPrepareWeapon[])( Work * ) = {
    FRT_ActPrepareStart, FRT_ActMotionOnce, FRT_ActStillStart, FRT_ActThink, NULL
} ;


/***

  移動アクションセット

  ***/
/*歩く*/
static void (*FRT_ActionWalk[])( Work * ) = {
    FRT_ActTurnToWalkStart, FRT_ActMotionOnce,/*向き変え*/
    FRT_ActWalkStart, FRT_ActWalk,
    FRT_ActTurnToStopStart, FRT_ActTurnToStop,/*向き変え*/
    FRT_ActStillStart, FRT_ActThink, NULL
} ;

static void (*FRT_ActionSideStepWalk[])( Work * ) = {
    FRT_ActSideStepWalkStart, FRT_ActSideStepWalk, FRT_ActStillStart, FRT_ActThink, NULL
} ;

#if 0
/* まず, マガジンを見て 白？マーク それから, 迷い構えを行なう */
static void (*FRT_ActionMagazineThrown[])( Work * ) = {

    FRT_ActWaitHalfSecStart, FRT_ActStandFirePStart, FRT_ActFireP,
    FRT_ActOnceMoreMagazineStart, FRT_ActFireP, FRT_ActEndOfMagazineStart,
    FRT_ActStillStart, FRT_ActThink, NULL
} ;
#endif

static void (*FRT_ActionTwoSecFaceWait[])( Work * ) = {
    FRT_ActFaceToPlayerStart, FRT_ActWaitTwoSecStart, FRT_ActWait, FRT_ActThink, NULL
} ;							  
