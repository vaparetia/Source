#ifndef _map_config_h_
#define _map_config_h_

#define MAP_ACT_LEVEL  GV_ACTOR_MANAGER

/*
 * レイアウトデータ作成時の各種パラメータ
 */
#define MAP_CHANL    4
#define MAP_PRIORITY 1
#define MAP_FLAGS    SPR_FLAG_PRIV

#define MAP_MASKPRI  0

#define MAP_ALPHA_MAX 64

/*
 * デフォルト値
 */
#define MAP_FADE_TIME 90   /* フェードイン / アウトに要する時間 */

#define MAP_SIGNAL_QUIT  0x0001  /* マップ表示終了シグナル */



/*
 * 表示のためのアクション名
 */

#include "map_action.h"

void * NewAllMapChild(int name, int where);
void * NewAllMapChildP(int layout_name, int pos_action);

#endif /* _map_config_h_ */
