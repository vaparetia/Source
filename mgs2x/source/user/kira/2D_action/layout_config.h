#ifndef _layout_config_h_
#define _layout_config_h_


/*
 * システム全体との兼ね合い
 */
#define LAYOUT_ACT_LEVEL   GV_ACTOR_MANAGER  /* Actor の実行レベル      */
#define LAYOUT_PRIO        0xf0              /* 優先順位                */
#if 0 // 実験的に試してみる
#undef LAYOUT_ACT_LEVEL
#undef LAYOUT_PRIO
#define LAYOUT_ACT_LEVEL   GV_ACTOR_DAEMON2  /* Actor の実行レベル      */
#define LAYOUT_PRIO        0x00              /* 優先順位                */
/* この設定にするとマネージャー起動と同時にlayout2dオブジェクトを使用すると失敗するので */
/* 起動時に初期化を行うコードを追加するように改良する必要がある */
#endif

#define LAYOUT_MAX        32  /* 管理することが可能なレイアウトデータ数 */

#define MAX_TEXTURES      16  /* 登録できる最大のテクスチャ数           */

#endif /* _layout_config_h_ */
