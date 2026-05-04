#ifndef _node_config_h_
#define _node_config_h_

#define NodeID           3721445   /* "node" (これ以外の chara 名称は不許可) */

#define NODE_ACT_MODE    GV_ACTOR_MANAGER  /* 動作レベル */
#define NODE_ACT_PRIO    0xf0              /* 優先順位   */

#define REP_MAX          ((300 / 2) / TIME_BASE)


/* ノード画面でレイアウトを使用する場合に定義 */
#define NODE_LAYOUT
/* #undef NODE_LAYOUT */

/* デバッグ用テキストを表示しない場合に定義 */
#define NO_TEXT
#undef NO_TEXT

/* 難易度設定を使用しない */
#define NO_DIFFICULTY
/* #undef NO_DIFFICULTY */


/*
 * 各挙動のアクション名
 */
#include "mode_act.h"   /* 画面の遷移に関連するアクション名定義           */
#if 0
#include "main_cur.h"   /* メインメニューの遷移に関連するアクション名定義 */
#include "opt_cur.h"    /* オプションメニューの遷移アクション名定義       */
#endif
#endif /* _node_config_h_ */
