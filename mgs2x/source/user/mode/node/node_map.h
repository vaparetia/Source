#ifndef _node_map_h_ 
#define _node_map_h_

#define NODEMAP_INIT   0x2dbee0    /* "初期描画" */
#define NODEMAP_CENTER 0x621ad7    /* "基準点"   */


#ifndef _node_map_c_
#define EXT extern
#else
#define EXT
#endif /* _node_map_c_ */



#undef EXT

/* Actor 起動関数プロトタイプ */
void * NewNodeMap(int name, int where);
void * NewNodeMap_for_Prog(int strcode);

#endif /* _node_map_h_ */
