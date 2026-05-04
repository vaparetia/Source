#ifndef _2d_data_h_
#define _2d_data_h_

/* =========================================================================
 * TRI 構成要素サマリ構造体
 * ========================================================================= */
typedef struct triTex {
  struct triTex * next;   /* リンク */
  char * tex_name;   /* テクスチャ名 */
} triTex;

typedef struct triInfo {
  struct triInfo * next;   /* リンク */
  char * tri_name;   /* tri 名称 */
  triTex * tex;
} triInfo;

/* ========================================================================= */

typedef enum {
  SP_EMPTY,
  SP_POINT,
  SP_LINE,
  SP_LINESTRIP,
  SP_BOX,
  SP_SPRITE,
  SP_ZOOM,
  SP_SPIN,
  SP_unknown
} sprID;

typedef struct {
  float x;
  float y;
} sprPos;

typedef struct {
  unsigned char r, g, b, a;
} sprRGBA;

/* 頂点情報構造体 */
typedef struct sprVertex {
  float x, y;                /* 座標値                                */
  unsigned char r, g, b, a;  /* RGBA値 */
  int  pos:1;                /* 頂点座標値指定の有無(0:無効/1:有効)   */
  int  col:1;                /* 頂点カラー値指定の有無(0:無効/1:有効) */
} sprVertex;

typedef enum {
  TR_WAIT,
  TR_SET,
  TR_MORF,
  TR_END
} sprOPCODE;

/*
 * 使用オブジェクト定義セクション
 */
typedef struct sprObject {
  struct sprObject * prev;      /* オブジェクトのリンク(直前) */
  struct sprObject * next;      /* オブジェクトのリンク(直後) */

  struct sprObject * parent;    /* 親オブジェクトのポインタ */

  int                serial_id; /* オブジェクトの通し番号   */
  char             * label;     /* オブジェクト名称のラベル */
  int                code;      /* 名称の StrCode           */

  sprID              obj_id;    /* オブジェクト種別 ID      */

  int                vertex_num;/* 頂点数                   */
  int                rgba_num;  /* RGBA 値の指定数          */

} sprObject;


/*
 * 状態定義セクション / 管理ブロック
 */
typedef struct sprStatus {
  struct sprStatus * prev;  /* 管理ブロックへのリンク(直前)          */
  struct sprStatus * next;  /* 管理ブロックへのリンク(直後)          */

  char      * label;        /* 状態の名称                            */
  int         code;         /* 状態の名称の StrCode                  */

  char      * tex_name;     /* テクスチャ名称                        */
  int         tex_code;     /* テクスチャ StrCode                    */

  /* 表示サイズ(SP_SPRITE の場合のみ使用) */
  float       width;
  float       height;

  float       u, v;     /* UV 値                            */
  float       uw, vh;   /* 正規化テクセルサイズ             */

  float       magni;    /* 下位オブジェクトの座標値係数     */

  /* 回転オブジェクトの回転値 */
  float       center_x;  /* 中心 x */
  float       center_y;  /* 中心 y */
  float       angle;     /* 回転角 */

  int         alpha;    /* アルファブレンディング関数指定値 */
  int         v_alpha;  /* α値(C が FIX の場合のみ)        */

  int         pri;      /* プライオリティ補正値(基準値に加算) */

  int         vertex_index;
  sprVertex * vertex;   /* 頂点ごとの情報                   */

  int         tex_sw:1; /* テクスチャの有無(0:無し/1:あり)  */
  int         disp:1;   /* 表示/非表示フラグ                */

} sprStatus;


typedef struct sprStatBlock {
  struct sprStatBlock * prev;
  struct sprStatBlock * next;

  enum {
    MODE_REFTRI,
    MODE_STATUS
  } mode;

  union {
    struct {
      char * label;
      int    code;
    } tri;
    struct {
      sprObject * target;     /* 対象オブジェクトのポインタ */
      sprStatus * begin;      /* 所属する状態構造体リンクの開始 */
      sprStatus * end;        /* 所属する状態構造体リンクの終端 */
    } st;
  } d;
} sprStatBlock;


typedef struct sprTrack {
  struct sprTrack * prev;
  struct sprTrack * next;

  sprObject * target;  /* トラックが操作対象とするオブジェクトのポインタ */

  int    seq_step;   /* トラックシーケンスのステップ数 */
  struct track_code {
    sprOPCODE   op;       /* シーケンス種別                    */
    int         v_time;   /* 時間値                            */
    sprStatus * v_stat;   /* 状態                              */
    int         spin:1;   /* 回転方向(0:時計方向/1:反時計方向) */
  } * seq;

} sprTrack;

/*
 * アクション定義ブロック
 */
typedef struct sprAction {
  struct sprAction * prev;
  struct sprAction * next;

  char              * label;      /* アクション名           */
  int                 code;       /* アクション名の StrCode */

  int                 track_nums; /* トラック数             */

  sprTrack          * begin;      /* アクションが保持するトラック(開始) */
  sprTrack          * end;        /* アクションが保持するトラック(終端) */
  
} sprAction;


/*
 * 読み出した情報を総括する構造体
 */
typedef struct sprLayout {

  int         obj_last_id;

  /*
   * 登録済オブジェクトのリンク
   */
  sprObject * objBegin;
  sprObject * objEnd;
  
  /*
   * 登録済状態ブロックのリンク
   */
  sprStatBlock * blockBegin;  /* 始端 */
  sprStatBlock * blockEnd;    /* 終端 */
  sprStatBlock * blockTarget; /* 編集対象 */
  
  /*
   * 登録済アクションブロックのリンク
   */
  sprAction * actBegin;  /* 始端     */
  sprAction * actEnd;    /* 終端     */
  sprAction * actTarget; /* 編集対象 */

  /*
   * .tri 生成情報(テクスチャを使用しない場合は NULL)
   */
  triInfo   * tri_info;

} sprLayout;

#define MALLOC(_siz)   my_malloc(_siz)
#define FREE(_ptr)     free(_ptr)
#define REALLOC(_ptr, _siz)   realloc(_ptr, _siz)
#define STRDUP(_str)  my_strdup(_str)

#ifdef _2d_data_c_
#define EXT
#else
#define EXT extern
#endif /* _2d_data_c_ */

/*
 * オブジェクト定義ワーク
 */
EXT sprLayout * spr_layout;

#undef EXT

int strcode(char * str);
sprLayout * sprReadLayoutScript(FILE * rfp);
void        sprDestroyReadData(sprLayout * layout);

int         tri_add_texture(char * tex_name);

sprObject * object_2d_new(char * name);
sprObject * object_2d_set_parent(sprObject * obj, char * parent);
sprObject * object_2d_set_class(sprObject * obj, sprID id);
sprObject * object_2d_set_vertex(sprObject * obj, int num);

sprStatBlock * statblock_2d_tri(char * tri_name);
sprStatBlock * statblock_2d_new(char * obj_name);

sprStatus * mode_2d_new(char * name);

sprAction * action_2d_new(char * name);

sprTrack  * track_2d_new(char * name);

sprTrack  * track_2d_gencode(sprOPCODE op, int v_time, char * form, int spin);

void      * my_malloc(size_t siz);
char      * my_strdup(char * str);
void        l2derror(void);

#endif /* _2d_data_h_ */
