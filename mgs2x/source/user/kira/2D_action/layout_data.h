#ifndef _layout_data_h_
#define _layout_data_h_

#include "sprite_2d.h"
#include "layout_config.h"

/*
 * 状態コマンド定義
 */
#define STAT_CMD_MASK   0xf0000000   /* コマンド部分を抽出するマスク */

#define STAT_CMD_END    0x00000000   /* 状態定義シーケンス終了     */
#define STAT_CMD_DISP   0x10000000   /* 表示状態指定               */
#define STAT_CMD_TEX    0x20000000   /* 使用テクスチャ指定         */
#define STAT_CMD_VERTEX 0x30000000   /* 頂点座標値指定             */
#define STAT_CMD_RGBA   0x40000000   /* 頂点 RGBA値指定            */
#define STAT_CMD_SIZE   0x50000000   /* 表示サイズ指定             */
#define STAT_CMD_UV     0x60000000   /* UV値指定                   */
#define STAT_CMD_UVSIZE 0x70000000   /* テクスチャサイズ指定       */
#define STAT_CMD_ALPHA  0x80000000   /* アルファブレンディング指定 */
#define STAT_CMD_PRI    0x90000000   /* プライオリティ補正値       */
#define STAT_CMD_CENTER 0xa0000000   /* 中心点指定                 */
#define STAT_CMD_ANGLE  0xb0000000   /* 回転角指定                 */
#define STAT_CMD_MAGNI  0xc0000000   /* 下位オブジェクト座標値係数 */
#define STAT_CMD_PARTUV 0xd0000000   /* 分数指定 uv 値             */
#define STAT_CMD_PTXSIZ 0xe0000000   /* 分数指定テクスチャサイズ   */

/*
 * アクションコマンド定義
 */
#define ACT_CMD_MASK    0xf0000000   /* コマンド部分を抽出するマスク */

#define ACT_CMD_END     0x00000000   /* 再生トラック終了       */
#define ACT_CMD_WAIT    0x40000000   /* 時間待ち               */
#define ACT_CMD_SET     0x80000000   /* 状態強制移行           */
#define ACT_CMD_MORF    0xc0000000   /* 状態モーフィング       */
#define ACT_CMD_SIGN    0x10000000   /* 埋め込みシグナル       */


#define OLA_VERSION_HD  0x10000000   /* Widescreen Flag        */

typedef struct l2dVersion_s {
// BP - l_ver was unused  unsigned int  l_ver;
// BP - c_ver[0] was reserved, so now we use it to mark endian swapping
//      c_ver[1..3] are used for the layout version
   unsigned char c_ver[4];
} l2dVersion;

/* データバージョンチェックマクロ */
#define VER_CHECK(ver, major, minor, patch) \
          ((((major) << 16) + (minor << 8) + (patch)) <= \
          (((int)(ver)->c_ver[1] << 16) + \
          ((int)(ver)->c_ver[2] << 8) + ((int)(ver)->c_ver[3])))
					     
#define BP_L2D_NEEDS_ENDIAN_SWAP(ver) ( (ver)->c_ver[0] == 0 )
#define BP_L2D_MARK_SWAPPED(ver) (ver)->c_ver[0] = 1

typedef struct l2dVertex {
  float x, y;               /* 座標値                         */
  unsigned char r, g, b, a; /* RGBA値                         */
  int   vert:1, rgba:1;     /* 設定されているかどうかのフラグ */
} l2dVertex;


typedef struct l2dStatus {
  int             code;           /* 状態に与えられた名前の StrCode         */

  /*
   * オブジェクトの状態をあらわす各種変数
   */
  /* テクスチャ関連 */
  int             tex_code;       /* テクスチャの StrCode                   */
  int             tex_handle;     /* テクスチャのハンドル                   */
  float           pos_u,  pos_v;  /* テクスチャの UV 位置(正規化)           */
  float           size_u, size_v; /* テクスチャのテクセルサイズ比           */

  /* 頂点情報 */
  l2dVertex     * vertex;         /* 頂点座標、頂点RGBA値                   */

  /* ※頂点情報の配列の大きさは、対応するオブジェクトの
       操作可能な頂点数と同じ。 */

  /* 基本表示情報 */
  float           size_w, size_h; /* 表示サイズ(Sprite の場合のみ使用)      */
  int             pri_adj;        /* プライオリティ補正値                   */

  u_long64   alpha;          /* アルファがある場合の、ALPHA レジスタ値 */

  float           magni;          /* 下位オブジェクトの座標値係数           */

  float           center_x;
  float           center_y;
  float           angle;

  int             alpha_flg:1;    /* 0:アルファなし / 1: アルファあり       */
  int             disp:1;         /* 0:非表示       / 1:表示                */
  int             h_rev:1;        /* 水平反転フラグ */
  int             v_rev:1;        /* 垂直反転フラグ */
  
} l2dStatus;

typedef struct l2dSprite {
  int              code;       /*オブジェクトのStrCode(Ver0.1.0未満は無意味)*/
  unsigned short   id;         /* オブジェクト ID    */
  SPR_OBJ        * obj;        /* オブジェクトの実体 */
  int              vertex_num; /* 座標指定可能な頂点の数 */
  l2dVertex      * vertex;     /* 正式な頂点座標値
				  (補間中も、補間が終るまで変更されない) */

  int              pri_base;   /* オブジェクトのベースプライオリティ     */
  int              pri_adjust; /* 現在のプライオリティ補正値             */
  int              tex_code;   /* 正式なテクスチャコード                 */
  int              tex_handle; /* 正式なテクスチャハンドル               */
  SPR_FIX          u, v, w, h; /* 正式なテクスチャの UV パラメータ       */

  float            width, height; /* 正式な表示サイズ(Sprite/Spin の場合のみ
				     使用) */

  float            magni;      /* 現在の子の座標値係数                  */

  int              spin_dir;   /* 正式な回転方向 */
  /*
    ※ 補間中は vertex は変動しないが、
       補間終了時に新たな状態の座標値に変更される。
  */

  int              now_status; /* 現在の状態番号                       */

  
  short            status_num; /* このオブジェクトに与えられた状態の数 */
  l2dStatus      * stat;       /* このオブジェクトが保持する状態       */

  int  (*conv_func)(struct l2dSprite * spr, int form1, int form2, float rate);

} l2dSprite;

typedef struct l2dTrack {
  l2dSprite    * target;     /* そのトラックが操作対象とするオブジェクト */
  unsigned int * steps_le;      /* トラック命令シーケンスへのポインタ       */ // BP - Added _le for Little Endian 

  int         pc;         /* 現在実行中のシーケンスへのインデックス   */ 
  int         cmd;        /* 実行中のコマンド                         */

  int         form_old;   /* 現在/変化前の状態                        */
  int         form_new;   /* 変化対象となる状態                       */

  int         time_limit; /* 変化/待ち時間                            */
  int         time_cnt;   /* 経過時間                                 */
  int         spin_dir;   /* 回転方向(Spin オブジェクトのみ)          */
} l2dTrack;

typedef struct l2dAction {
  int        code;        /* アクション名                         */
  int        tracks_num;  /* そのアクションが保持するトラックの数 */
  l2dTrack * tracks;      /* トラック構造体へのポインタ           */

  int        finished_cnt;/* 再生が終ったトラックの数             */
} l2dAction;

/*
 * オブジェクト、状態、アクションの各セクションを統括して管理するための構造体
 */
typedef struct l2dData {
  /* データ形式のバージョン */
  l2dVersion  ver;

  /* 基準プライオリティ値   */
  int         base_pri;

  /* 表示オブジェクトの追加フラグ */
  int         additional_flag;

  /* 新規登録テクスチャ数 */
  int         texture_cnt;
  int         tex_handles[ MAX_TEXTURES ];

  /* オブジェクト定義域 */
  int         obj_nums;
  l2dSprite * obj;

  /* アクション定義域 */
  int         act_nums;
  l2dAction * act;

  /* Used to indicate whether or not the layout file has been exported with widescreen coordinates */
  int         widescreen;

  float       xsize;    // coordinate system used by the layout - we'll convert all coords to screen coords based on these values when loading
  float       ysize;

  /* 状態は、各オブジェクト単位で保持される */
} l2dData;



#ifndef _layout_data_c_
#define EXT extern
#else
#define EXT
#endif /* _layout_data_c_ */


#undef EXT


l2dSprite * l2dSpriteInit(l2dSprite * spr);
l2dData   * l2dObjectDefine(l2dData * dat, void * ptr, int chanl);
l2dData   * l2dObjectStatus(l2dData * dat, void * ptr, float safeZoneOffsetY);
l2dData   * l2dAdditionalFlag(l2dData * dat, int add_flag);
l2dData   * l2dObjectAction(l2dData * dat, void * ptr);
l2dData   * l2dSetupData(void * entry_ptr, int chanl, int base_pri, int add_flag, float safeZoneOffsetY);
void        l2dFreeData(l2dData * dat);

SPR_OBJ   * l2dGetObject(l2dData * dat, int code);
void      * l2dGetParts(l2dData * dat, int code);

#endif /* _layout_data_h_ */
