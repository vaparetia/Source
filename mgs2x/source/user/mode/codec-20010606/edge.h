#ifndef _edge_h_
#define _edge_h_

#define Qsize(_v)   (sizeof(_v) / sizeof(u_long128))

#define EDGE_DEPTH      32     /* ぼかし段階数       */
#define EDGE_BASE_ALPHA 128    /* 中央部のアルファ値 */

#define EDGE_R  128
#define EDGE_G  128
#define EDGE_B  128


/*
 * 顔表示窓の描画されている領域のフレームバッファに、
 * 適切なアルファ値を書き込むパケット
 * 
 * 表示領域の中央部が最大(128), 端にいくほど小さくなり、最終的に 0 になる。
 *
 * ※本構造体には、DMAtag はふくまれていないため、注意。
 */
typedef struct {
  DG_GIFTAG  giftag;
  struct _edge_data {

    DG_GSREG   test0;  /* デプステスト設定(常に合格)      */
    DG_GSREG   zbuf0;  /* z バッファを更新(zの値は均一に) */
    DG_GSREG   alpha;  /* アルファブレンディング設定      */
    DG_GSREG   tex0;   /* テクスチャマッピング情報        */

    /* Sprite で、Z値および中央部のアルファを初期化する */
    DG_GSREG   spr_prim;
    DG_GSREG   rgbq;
    DG_GSREG   uv0, xyz0;  /* 左上 */
    DG_GSREG   uv1, xyz1;  /* 右下 */
    
    /* LineStrip で Box を描画し、周囲のアルファを段階的に変更する */
    DG_GSREG   box_prim;
    struct {
      DG_GSREG   rgbq;
      DG_GSREG   texa;
      DG_GSREG   uv0, xyz0;   /* 左上(始点) */
      DG_GSREG   uv1, xyz1;   /* 右上       */
      DG_GSREG   uv2, xyz2;   /* 右下       */
      DG_GSREG   uv3, xyz3;   /* 左下       */
      DG_GSREG   uv4, xyz4;   /* 左上(終点) */

    } box[ EDGE_DEPTH ];

    DG_GSREG   test1;
    DG_GSREG   zbuf1;
  } data;
} FaceEdge ALIGN16;

#ifndef _edge_c_
#define EXT   extern
#else
#define EXT
#endif /* _edge_c_ */

/*
 * グローバル変数 
 */






#undef EXT





/*
 * プロトタイプ宣言
 */
int edgeInitFaceEdgePacket(FaceEdge * packet, int chanl, u_long64 tex0,
			   int x_base, int y_base,
			   int u_base, int v_base,
			   int x0, int y0, int x1, int y1, int z);

#endif /* _edge_h_ */
