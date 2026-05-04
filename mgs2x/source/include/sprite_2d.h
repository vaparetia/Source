/*
  sprite_2d.h
  2D表示系ライブラリ用ヘッダファイル
*/
#ifndef _sprite_2d_h_
#define _sprite_2d_h_

#define SPR_MOST_NEAR  -1   /* 多分使ってないと思うけど,
			       あまり意味が無い盲腸みたいなマクロ */


/* 実数などを、スプライト座標値に変換するマクロ */
#if 0 //BP_PS2 def PSX2
/* --- PS2 --- */

#define SPR_FIXED(n)   ((int)((float)(n) * 16.0F))

/* 物理ピクセル座標を、概念座標値に変換するマクロ */
#define SPR_PIX_X(chanl, x) ((float)(x) * 512.0f / (float)DG_Chanls[(chanl)].width)
#define SPR_PIX_Y(chanl, y) ((float)(y) * 384.0f / (float)DG_Chanls[(chanl)].height)

/* 概念座標値を物理ピクセル座標に変換するマクロ */
#define SPR_SCALE_X(chanl, x)  ((int)((float)DG_Chanls[(chanl)].width * (x) / 512.0f))
#define SPR_SCALE_Y(chanl, y)  ((int)((float)DG_Chanls[(chanl)].height * (y) / 384.0f))

typedef int  SPR_FIX;   /* 固定小数点座標値(上位12bit整数部,下位4bit小数部) */

/* --- PS2 end --- */
#else
/* --- XBOX --- */

#define SPR_FIXED(n)   ((float)(n))

/* 物理ピクセル座標を、概念座標値に変換するマクロ */
#define SPR_PIX_X(chanl, x) ((float)(x) * 512.0 / (float)DG_Chanls[(chanl)].width)
#define SPR_PIX_Y(chanl, y) ((float)(y) * 384.0 / (float)DG_Chanls[(chanl)].height)

/* 概念座標値を物理ピクセル座標に変換するマクロ */
#define SPR_SCALE_X(chanl, x)  (((float)DG_Chanls[(chanl)].width * (x) / 512.0))
#define SPR_SCALE_Y(chanl, y)  (((float)DG_Chanls[(chanl)].height * (y) / 384.0))

typedef float  SPR_FIX;   /* 固定小数点座標値(上位12bit整数部,下位4bit小数部) */

/* --- XBOX end --- */
#endif

/*
 * 必要な各種データ型
 */
typedef struct SPR_COLOR {
#if BPE_IS_ENDIAN_BIG()
   unsigned char a;
   unsigned char b;
   unsigned char g;
   unsigned char r;
#else
   unsigned char r;
   unsigned char g;
   unsigned char b;
   unsigned char a;
#endif
} SPR_COLOR;  /* 色情報 */

#if BPE_IS_ENDIAN_BIG()
#  define SPR_COLOR_CTOR(r,g,b,a) { a, b, g, r }
#else
#  define SPR_COLOR_CTOR(r,g,b,a) { r, g, b, a }
#endif

typedef int SPR_PRI;  /* プライオリティ値 */

/* テクスチャ関係の情報について */
typedef struct SPR_TEX {
  SPR_FIX      u, v;     /* テクスチャを取得する u, v 座標(左上)     */
  SPR_FIX      w, h;     /* 使用するテクスチャビットマップの幅と高さ */

  SPR_FIX      pu, pv;   /* 本来の UV 値                             */
  SPR_FIX      pw, ph;   /* 本来の幅,高さ(テクセル単位)              */

  int          flags;    /* テクスチャの扱いを決めるフラグ           */
  int          pabe;     /* PABE レジスタに設定する値                */
  int          tri;      /* 対応する TRI のハンドル                  */
  DG_TEX     * dgtex;    /* 対応する DG_TEX 構造体へのポインタ       */
} SPR_TEX;


/*
 * スプライト種別識別 ID
 */
typedef enum {
  SP_EMPTY,    /* 空プリミティブ(何も表示されない) */
  SP_POINT,    /* 点                */
  SP_LINE,     /* 線分              */
  SP_LINESTRIP,/* 折れ線            */
  SP_BOX,      /* 長方形            */
  SP_SPRITE,   /* 等倍矩形          */
  SP_ZOOM,     /* 拡大縮小          */
  SP_SPIN,     /* 回転              */
  SP_POLY,     /* 4頂点任意指定変形 */
  SP_TRIANGLE, /* 三角形            */
  SP_CLIP,     /* クリッピング      */
  SP_SPRITE_F, /* added by Steve for BP */
  SP_ZOOM_F,
  SP_SPIN_F,
  SP_BOX_F,
  SP_POLY_F,
} SPR_ID;

/*
 * 座標構造体
 */
typedef struct SPR_POS {
  float x;
  float y;
} SPR_POS;

/*
 * 領域構造体
 */
typedef struct SPR_RECT {
  SPR_POS  begin;
  SPR_POS  end;
} SPR_RECT;

union SPR_OBJ;

/*
 * スプライト管理データ構造体
 */
typedef struct SPR_HEADER {
  union SPR_OBJ * parent;  /* 親オブジェクト         */
  union SPR_OBJ * child;   /* 子オブジェクトの末端   */
  union SPR_OBJ * next;    /* 次の兄弟オブジェクト   */
  union SPR_OBJ * prev;    /* 直前の兄弟オブジェクト */

  void  (*func_free) (void * ptr); /* オブジェクトメモリ領域開放関数ポインタ */

  SPR_ID       id;     /* スプライト種別判別用 ID フィールド        */
  SPR_PRI      pri;    /* プライオリティ値(3D の 描画 Z 値と同基準) */
  int          flags;  /* 描画上の各種フラグ                        */
  unsigned short chanl;  /* 表示チャネル(root object のみ意味を持つ)  */
  unsigned short name;   /* オブジェクト ID (表示上は特に関知しない)  */
  u_long64  alpha;  /* アルファブレンディングの際の ALPHA レジスタ値 */

  /* DMA 転送用パラメタ領域 */
  void       * packet;   /* パケットバッファ                      */
  void       * pk[2];    /* パケット0, 1 のポインタ               */
  int          q_size;   /* パケットの qword サイズ               */

  float        scale;    /* 子の座標値に対する係数                */
} SPR_HEADER;

typedef struct SPR_HEADER_EX {
  SPR_HEADER      head;
  SPR_TEX         tex;
} SPR_HEADER_EX;

/*
 * Empty
 *
 * 何も表示されない 2D オブジェクト。
 * 親子関係を利用して、複数のオブジェクトをまとめる場合などに使用する。
 * (というか、それ以外の用途に使おうとしても無意味)
 */
typedef struct SPR_EMPTY {
  SPR_HEADER  head;   /* 管理データ */  
  SPR_POS     pos;    /* 座標点     */
} SPR_EMPTY;


/*
 * 2次元 点
 *
 * 任意座標に任意カラーの点を表示する。
 */
typedef struct SPR_POINT {
  SPR_HEADER  head;   /* 管理データ */
  SPR_POS     pos;    /* 座標値 */
  SPR_COLOR   col;    /* 色情報 */
} SPR_POINT;

/*
 * 2次元線分
 *
 * 任意2点を結ぶ線分を描画
 */
typedef struct SPR_LINE {
  SPR_HEADER  head;         /* 管理データ */
  SPR_POS     pos[2];       /* 座標値(始点、終点) */
  SPR_COLOR   col[2];       /* 色情報(始点、終点) */
} SPR_LINE;

/*
 * 折れ線
 *
 * 連結された線分を描画
 */
typedef struct SPR_LINESTRIP {
  SPR_HEADER          head;   /* 管理データ           */
  
  int                 pcnt;   /* 頂点の数             */
  void * (* func_malloc)(long64 size);  /* 頂点領域確保関数 */

  /* 頂点とその色のリスト */
  struct spr_lstrip_pos {
    SPR_POS   pos;    /* 頂点座標 */
    SPR_COLOR col;    /* 頂点の色 */
  } * plist;
} SPR_LINESTRIP;

/*
 * 長方形
 *
 * 任意2点を対角とする長方形を描画
 */
typedef struct SPR_BOX {
  SPR_HEADER  head;
  SPR_RECT    rect;         /* 管理データ */
  SPR_COLOR   col;          /* 色情報     */
} SPR_BOX;


/*
 * 等倍矩形スプライト(もっともシンプルなスプライト)
 */
typedef struct SPR_SPRITE {
  SPR_HEADER_EX  head;         /* 管理データ(テクスチャ情報つき) */
  SPR_POS        pos;          /* 左上の座標値                   */
  SPR_COLOR      col;          /* 頂点 RGBA 値(全頂点共通)       */
  float          dw, dh;       /* 表示幅、高さ                   */
} SPR_SPRITE;


/*
 * 拡大縮小機能つきスプライト
 */
typedef struct SPR_ZOOM {
  SPR_HEADER_EX  head;   /* 管理データ(テクスチャ情報つき) */
  SPR_RECT       rect;   /* 左上、右下の座標値             */
  SPR_COLOR      col;    /* 左上、右下の RGBA 値           */
} SPR_ZOOM;

/*
 * 回転機能つきスプライト
 */
typedef struct SPR_SPIN {
  SPR_HEADER_EX  head;         /* 管理データ         */
  SPR_POS        pos;          /* 表示位置           */
  SPR_POS        center;       /* 回転の中心         */
  SPR_COLOR      col;          /* 頂点 RGBA 値       */
  float          dw, dh;       /* 表示サイズ         */
  float          spin;         /* 回転角[rad]        */
} SPR_SPIN;

/*
 * 任意4頂点スプライト
 */
typedef struct SPR_POLY {
  SPR_HEADER_EX  head;         /* 管理データ         */
  /*
    頂点座標および RGBA 値は、左上から時計周りに指定する
  */
  SPR_POS        vert[4];   /* 各頂点座標            */
  SPR_COLOR      col[4];    /* 各頂点 RGBA 値        */
} SPR_POLY;

typedef struct SPR_TRIANGLE {
  SPR_HEADER_EX  head;
  SPR_POS        vert[3];
  SPR_COLOR      col[3];
} SPR_TRIANGLE;

/*
 * クリッピングオブジェクト
 */
typedef struct SPR_CLIP {
  SPR_HEADER     head;
  SPR_POS        pos;          /* 領域左上座標                 */
  float          dw, dh;       /* 幅、高さ                     */
} SPR_CLIP;

/*
 * ポインタ用に共通の型が必要な場合の共用体
 *
 * 実際にこの型を使ってメモリが確保されることはない…といいなぁ。
 */
typedef union SPR_OBJ {
  SPR_HEADER    head;    /* 管理データを得たいときに使用     */
  SPR_HEADER_EX ex_hd;   /* テクスチャ情報を得たいときに使用 */
  SPR_EMPTY     empty;
  SPR_POINT     point;
  SPR_LINE      line;
  SPR_LINESTRIP linestrip;
  SPR_BOX       box;
  SPR_SPRITE    sprite;
  SPR_ZOOM      zoom;
  SPR_SPIN      spin;
  SPR_POLY      poly;
  SPR_TRIANGLE  triangle;
} SPR_OBJ;


/*
 * フラグビットの定義
 */
/* ステータス */
#define SPR_FLAG_HIDDEN      0x8000   /* 非表示フラグ                       */
#define SPR_FLAG_NO_TEX      0x4000   /* テクスチャがロードされていない     */
#define SPR_FLAG_PRIV        0x2000   /* 特権モードでも描画する             */

/* 表示属性 */
#define SPR_FLAG_H_REV       0x0040   /* 左右反転                           */
#define SPR_FLAG_V_REV       0x0080   /* 上下反転                           */

/* 表示効果 */
#define SPR_FLAG_ALPHA       0x0001   /* アルファブレンディングを行う       */
#if 1 //BP_XBOX def KP_XBOX
#define SPR_FLAG_NOALPHATEST 0x0002   /* アルファテストを行わないフラグ     */
#endif

#define SPR_FLAG_ROTATE_MASK 0x0300
#define SPR_FLAG_ROTATE_0    0x0000
#define SPR_FLAG_ROTATE_90   0x0100   // used for bomb textures - rotate the sprites 0, 1, 2, or 3 x90 degrees clockwise
#define SPR_FLAG_ROTATE_180  0x0200
#define SPR_FLAG_ROTATE_270  0x0300

/* スプライトの表示状態を切替えるマクロ */
#define SPR_SHOW(prim)  (prim)->head.flags &= ~SPR_FLAG_HIDDEN
#define SPR_HIDE(prim)  (prim)->head.flags |= SPR_FLAG_HIDDEN

/* 子スプライト座標値係数を設定するマクロ */
#define SPR_MAG(prim, _mag)  (prim)->head.scale = (_mag)


/* スプライトに Alpha レジスタ値を設定するマクロ */
#define SPR_ALPHA(prim, a)  (prim)->head.alpha = (a)

/* 各種フラグを設定するマクロ */
#define SPR_SET_FLAGS(prim, f)  (prim)->head.flags |= (f)

/* スプライト画面のサイズ */
#define SPR_SCRN_WIDTH  512.0F
#define SPR_SCRN_HEIGHT 384.0F

/* プライオリティの最大値,最小値 */
#define SPR_PRI_MOST_NEAR   7
#define SPR_PRI_MOST_FAR    0


#ifndef _sprite_2d_c_
#define EXT extern
#else
#define EXT
#endif



#undef EXT

/*
 * 関数プロトタイプ宣言
 */
/* LineStrip の頂点数設定 */
int SPR_SetLineStripVertexNumber(SPR_OBJ * prim, int num);

/* 座標設定系 */
int SPR_SetPosEmpty(SPR_OBJ * prim, SPR_POS * pos);
int SPR_SetPosPoint(SPR_OBJ * prim, SPR_POS * pos);
int SPR_SetPosLine(SPR_OBJ * prim, SPR_POS * begin, SPR_POS * end);
int SPR_SetPosLineStrip(SPR_OBJ * prim, int begin, int num, SPR_POS * pos);
int SPR_SetPosBox(SPR_OBJ * prim, SPR_RECT * rect);
int SPR_SetPosSprite(SPR_OBJ * prim, SPR_POS * pos);
int SPR_SetPosZoom(SPR_OBJ * prim, SPR_RECT * rect);
int SPR_SetPosSpin(SPR_OBJ * prim, SPR_POS * pos);
int SPR_SetPosPoly(SPR_OBJ * prim, SPR_POS * pos);
int SPR_SetPosTriangle(SPR_OBJ * prim, SPR_POS * pos);

int SPR_SetSizeSprite(SPR_OBJ * prim, float w, float h);
int SPR_SetSizeSpin(SPR_OBJ * prim, float w, float h);

int SPR_SetCenterSpin(SPR_OBJ * prim, SPR_POS * center); /* 回転の中心点 */
int SPR_SetRollSpin(SPR_OBJ * prim, float spin);             /* 回転角       */

/* 色指定系 */
int SPR_SetColorPoint(SPR_OBJ * prim, int r, int g, int b, int a);
int SPR_SetColorBox(SPR_OBJ * prim, int r, int g, int b, int a);
int SPR_SetColorLine(SPR_OBJ * prim, int p, int r, int g, int b, int a);
int SPR_SetColorLineStrip(SPR_OBJ * prim, int p, int r, int g, int b, int a);
int SPR_SetColorSprite(SPR_OBJ * prim, int r, int g, int b, int a);
int SPR_SetColorZoom(SPR_OBJ * prim, int r, int g, int b, int a);
int SPR_SetColorSpin(SPR_OBJ * prim, int r, int g, int b, int a);
int SPR_SetColorPoly(SPR_OBJ * prim, int p, int r, int g, int b, int a);
int SPR_SetColorTriangle(SPR_OBJ * prim, int p, int r, int g, int b, int a);

/*
 * 共通処理系
 */
int SPR_SetPriority(SPR_OBJ * prim, SPR_PRI pri); /* プライオリティ設定 */

/*
 * メモリ確保関数の指定
 */
int SPR_SetMemoryManager(void * (*func_malloc)(long64 size),
			 void (* func_free)(void * ptr));

int SPR_ResetMemoryManager(void);

/*
 * テクスチャ設定
 */
int SPR_LoadTexture(int code);
int SPR_KillTexture(int handle);

int SPR_ObjSetTexture(SPR_OBJ * prim, int code, int flags);

/*
 * GV_Malloc() を使用したメモリ確保と、初期化が一体になった
 * コンビニエンス関数のセット。こちらを用いることを推奨
 * (前処理、後処理が大変なので)
 */
SPR_OBJ * SPR_Create_2D_Object(SPR_ID id, int chanl, SPR_OBJ * parent);
int       SPR_Destroy_2D_Object(SPR_OBJ * prim);


void    * SPR_Malloc(size_t size);
void      SPR_Free(void * ptr);

void    * SPR_Init2D_ObjectDriver(void);


/* 2Dオブジェクト描画前描画設定関数(取扱注意) */
void SPR_InitPreDrawTag(int chanl);
int SPR_SetPreDrawTag(int chanl,
		      void **nxt1, void **nxt2,
		      void * packet1, void * packet2);

SPR_OBJ * SPR_DuplicateTree(SPR_OBJ * node);

/* int sprPrimSetParent(SPR_OBJ * prim, SPR_OBJ * parent); */

#endif /* _sprite_2d_h_ */
