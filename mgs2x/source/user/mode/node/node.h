#ifndef _node_h_
#define _node_h_


#define NAME_ENTRY_END 0x0010

#define NODE_FLAG_BLACK_IN   0x0001   /* 暗転状態から開始する */
#define NODE_FLAG_BLACK_OUT  0x0002   /* 暗転状態で終る       */

/*
 * 各オプション項目の番号定義
 */
enum {
  OPT_DIFFICULTY,
  OPT_VIBRATION,
  OPT_CAPTION,
  OPT_BLOOD,
  OPT_RADAR,
  OPT_OWN_VIEW,
  OPT_SOUND,
  OPT_QCHANGE,
  OPT_EXIT,

  OPT_INDEX_CNT  /* オプション項目数 */
};
#ifdef NO_DIFFICULTY
#define OPT_BASE 1
#else
#define OPT_BASE 0
#endif /* NO_FIFFICULTY */


#ifndef _node_c_
#define EXT extern
#else
#define EXT
#endif /* _node_c_ */


#undef EXT

void * NewNOdeDaemon(int name, int map);  /* ノード画面デーモン起動 */
int    NewNode(void);                     /* ノードにアクセス!!     */
int    NewNode_for_Prog(int map, int name, int flag);   /* ノードアクセス(プログラム用) */


#endif /* _node_h_ */
