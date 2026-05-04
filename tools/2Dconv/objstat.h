#ifndef _objstat_h_
#define _objstat_h_

typedef struct statPoint {
  int   flag;  /* 0:設定しない / 1:設定する */
  float x;
  float y;
} statPoint;

typedef struct statRGBA {
  int flag;
  unsigned char r, g, b, a;
} statRGBA;

typedef struct statStatus {
  struct statStatus * prev;
  struct statStatus * next;

  char * stat_name;   /* 状態名 */

  /* 表示 / 非表示の区別 */
  int disp;
  int alpha_flg;          /* アルファブレンディング指定       */
  unsigned long alpha;              /* ALPHA レジスタの値               */

  /* テクスチャの StrCode */
  int tex;     /* テクスチャを使用しない場合は -1 を設定する */

  /* 頂点数と、頂点情報 */
  int         vertex_num;     /* 頂点数                           */
  statPoint * vertex;         /* 頂点リスト                       */
  statRGBA  * rgba;           /* 頂点色(頂点数は頂点リストと同じ) */

  /* サイズ(sprite のみ) */
  float       width;
  float       height;
  
  /* UV 値 */
  float       u, v;
  /* テクスチャサイズ */
  float       uw, vh;
  
} statStatus;


typedef struct statBlock {
  struct statBlock * prev;
  struct statBlock * next;

  enum {
    MODE_REFTRI,
    MODE_STATUS
  } mode;

  union {
    struct {
      int tri_code;
    } tri;

    struct {
      int          id_obj;  /* 対象となるオブジェクトの ID */
      statStatus * stat_begin;
      statStatus * stat_end;
    } st;
  } d;
  
} statBlock;

typedef struct statStLink {
  statBlock * begin;
  statBlock * end;
  statBlock * now;
} statStLink;



#ifndef _objstat_c_
#define EXT extern
#define INIT(_n...)
#else
#define EXT
#define INIT(_n...) = _n
#endif /* _objstat_c_ */

EXT statStLink statLink INIT({NULL, NULL, NULL});

#undef EXT
#undef INIT(_n...)

int statInitObjectStatus(void);
int statRefTri(char * cmd, tknFILE * tkn, lblBlock * block, FILE *wfp);
int statDefStatus(char * cmd, tknFILE * tkn, lblBlock * block, FILE *wfp);



#endif /* _objstat_h_ */
