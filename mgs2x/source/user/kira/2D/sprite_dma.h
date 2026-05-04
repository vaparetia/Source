#ifndef _sprite_dma_h_
#define _sprite_dma_h_

#include "sprite_2d.h"

#define MAX_TRI      16   /* 使用できる .tri の最大数 */
#define MAX_CHANL    5    /* 描画対象となるチャネル数 */
#define SPR_PRI_NUM  8

/*
 * DMA パケット生成用定型構造体
 *
 * 各 2D プリミティブに合わせて、定型の構造体を定義しておく。
 */

/*
 * 各チャネル分の発端となる最初のパケット。
 */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG  dmatag;  
  DG_GIFTAG  giftag;
  DG_GSREG   test;
} spr_TransRoot ;

/*
 * 途中挿入するテクスチャ転送パケット
 */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG  call;      /* テクスチャ転送パケットを CALL する                */
  DG_DMATAG  texflush;  /* TEXFLUSH を行うパケットを REF で転送              */
  DG_DMATAG  nxt;       /* 次のパケットへ NEXT で移行                        */
} spr_TexTrans ;
  
typedef ALIGN16_DECL(struct) {
  DG_DMATAG  dmatag;
  DG_GIFTAG  giftag;
  DG_GSREG   test;
  DG_GSREG   pabe;
} spr_TexRecov ;

typedef ALIGN16_DECL(struct) {
  DG_GIFTAG  giftag; /* 多くの場合、ここで PRIM も送ってしまう */
  DG_GSREG   alpha;
} spr_Alpha ;

typedef struct {
  DG_GIFTAG giftag;
  struct _txat_data {
    DG_GSREG alpha;
    DG_GSREG pabe;
    DG_GSREG tex2;
#ifndef NO_TEX1
    DG_GSREG tex1;
#endif /* NO_TEX1 */
  } data;
} spr_TexAtt;

/*
 * Empty 描画用の DMAパケットは存在しない。そもそも描画されない
 */

/* Point 描画用のDMAパケット */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag;
  struct _gif_point {
    spr_Alpha alpha;

    DG_GIFTAG giftag;
    struct _data_point {
      /* 描画点 */
      u_long64 rgbq0;   /* 頂点色設定                 */
      u_long64 xyz0;    /* 頂点座標指定               */
    } data;
  } gif;
} spr_DrawPoint ;

/* Line 描画用の DMA パケット */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag;
  struct _gif_line{
    spr_Alpha alpha;
    DG_GIFTAG giftag;
    struct _data_line{
      /* 始点 */
      u_long64 rgbq0;
      u_long64 xyz0;

      /* 終点 */
      u_long64 rgbq1;
      u_long64 xyz1;
    } data;
  } gif;

} spr_DrawLine ;

/*
 * LineStrip 用の DMA パケット。
 *
 * ヘッダ部と頂点部に分けられている。
 */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag;
  struct _gif_lstrip {
    DG_GIFTAG giftag;  /* PRIM含む */
    struct _data_lstrip {
      DG_GSREG alpha;
    } data;
    DG_GIFTAG giftag_ver;
  } gif;
} spr_DrawLineStrip ;

typedef ALIGN16_DECL(struct) {
  u_long64  rgbq;
  u_long64  xyz;
} spr_DrawLS_pos ;

typedef ALIGN16_DECL(struct) {
  spr_DrawLineStrip  head;
  spr_DrawLS_pos     pos[0];
} spr_DrawLS_template ;

/* Box 描画用の DMA パケット */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag;
  struct _gif_box {
    spr_Alpha alpha;   /* ALPHA 設定用 */

    DG_GIFTAG giftag;
    struct _data_box {
      u_long64  rgbq;
      u_long64  xyz0;
      u_long64  xyz1;
      u_long64  xyz2;
      u_long64  xyz3;
      u_long64  xyz4;
      u_long64  dummy;
    } data;
  } gif;
} spr_DrawBox ;

/* Sprite 描画用の DMA パケット */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag;
  struct _gif_sprite {
    spr_TexAtt attrib;
    DG_GIFTAG giftag;
    struct _data_sprite {
      /* テクスチャがらみ */
      u_long64 clamp;
      u_long64 tex0;

      /* 左上 */
      u_long64 rgbq0;
      u_long64 uv0;
      u_long64 xyz0;

      /* 右下 */
      u_long64 rgbq1; 
      u_long64 uv1;
      u_long64 xyz1;
    } data;
  } gif;
} spr_DrawSprite ;



/* Zoom 描画用の DMA パケット */
typedef spr_DrawSprite spr_DrawZoom;
/* DMA パケットは、基本的に Sprite と同じ */

/* Spin 描画用の DMA パケット */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag;
  struct _gif_spin {
    spr_TexAtt attrib;  
    DG_GIFTAG giftag;
    struct _data_spin {
      /* テクスチャがらみ */
      u_long64 clamp;
      u_long64 tex0;

      /* 左上 */
      u_long64 rgbq0;
      u_long64 uv0;
      u_long64 xyz0;

      /* 右上 */
      u_long64 rgbq1;
      u_long64 uv1;
      u_long64 xyz1;

      /* 右下 */
      u_long64 rgbq2;
      u_long64 uv2;
      u_long64 xyz2;

      /* 左下 */
      u_long64 rgbq3;
      u_long64 uv3;
      u_long64 xyz3;
    } data;
  } gif;
} spr_DrawSpin ;

/* Poly 描画用の DMA パケット */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag;
  struct _gif_poly {
    spr_TexAtt attrib;  
    DG_GIFTAG giftag;
    struct _data_poly {
      /* テクスチャがらみ */
      u_long64 clamp;
      u_long64 tex0;

      /* 左上 */
      u_long64 rgbq0;
      u_long64 uv0;
      u_long64 xyz0;

      /* 右上 */
      u_long64 rgbq1;
      u_long64 uv1;
      u_long64 xyz1;

      /* 右下 */
      u_long64 rgbq2;
      u_long64 uv2;
      u_long64 xyz2;

      /* 左下 */
      u_long64 rgbq3;
      u_long64 uv3;
      u_long64 xyz3;
    } data;
  } gif;
} spr_DrawPoly ;

/* Triangle 描画用の DMA パケット */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag;
  struct _gif_triangle {
    spr_TexAtt attrib;  
    DG_GIFTAG giftag;
    struct _data_triangle {
      /* テクスチャがらみ */
      u_long64 clamp;
      u_long64 tex0;

      /* 左上 */
      u_long64 rgbq0;
      u_long64 uv0;
      u_long64 xyz0;

      /* 右上 */
      u_long64 rgbq1;
      u_long64 uv1;
      u_long64 xyz1;

      /* 左下 */
      u_long64 rgbq2;
      u_long64 uv2;
      u_long64 xyz2;
    } data;
  } gif;
} spr_DrawTriangle ;

#if 0
typedef ALIGN16_DECL(struct) _spr_SetClip {
  DG_DMATAG dmatag;
  struct _gif_clip {
    DG_GIFTAG giftag;
    struct _data_clip {
    } data;
  } gif; 
} spr_SetClip ;
#endif

typedef ALIGN16_DECL(struct) _spr_DMA_Packet {
  /* パケット構造体のチェインは、NULL で始端/終端を表す */
  struct _spr_DMA_Packet * prev;  /* 直前のパケット構造体 */
  struct _spr_DMA_Packet * next;  /* 次のパケット構造体   */

  DG_DMAPACK * dmapack;

  SPR_OBJ    * prim;
  int          pre_flag;   /* 直前に呼ばれた際のフラグ */
  int          num;
  union {
    spr_DrawPoint  point;
    spr_DrawLine   line;
    spr_DrawSprite sprite;
    spr_DrawZoom   zoom;
    spr_DrawPoly   poly;
  } draw[2];
} spr_DMA_Packet ;

/*
 * 関数プロトタイプ
 */
int sprCreate_DMA_Packet(SPR_OBJ * prim);
int sprRegistPrim(SPR_OBJ * prim);
int sprErasePrim(SPR_OBJ * prim);
int sprPrimUnref(SPR_OBJ * prim);

int spr_strip_root_object(SPR_OBJ * prim);
int spr_driver_active_status(void);

/* 現在の確保/開放関数で確保、開放を行う */
void * spr_malloc(long64 size);
void   spr_free(void * ptr);

/* 現在登録されている確保/開放関数のポインタを得る */
void * (* spr_get_func_malloc(void))();
void   (* spr_get_func_free(void))();

void * SPR_DefMalloc(long64 size);
void   SPR_DefFree(void * ptr);

#endif /* _sprite_dma_h_ */

