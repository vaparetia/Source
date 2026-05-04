#ifndef _cylinder_h_
#define _cylinder_h_

#define CYLINDER_ALPHA_SIGNAL       0x0001
#define CYLINDER_SPIN_CHANGE_SIGNAL 0x0002


#define F_PI   ((float)M_PI)

/*
 * プログラムインタフェースでパラメータを与える際の構造体
 */
typedef struct {
  int      alpha;    /* 最大α値                            */

  FVECTOR  O;        /* 起点                                */
  FVECTOR  rot;      /* 軸の回転角                          */

  float    r;        /* 半径                                */
  float    width;    /* 弧の長さ                            */
  float    height;   /* イメージの高さ                      */

  float    posy;     /* イメージを配置する起点からの y 位置 */

  int      tri_name; /* 使用する TRI                        */
  int      tex_name; /* 使用するテクスチャ名                */

  /*
   * アニメーションパラメータ
   */

  /* 軸廻り回転 */
  int      tm_spin;  /* 一周するのにかける時間              */
  float    mv_spin;  /* 初期角度                            */
  
  /* 軸沿い移動 */
  int      tm_slide;   /* 移動にかける時間   */
  float    mv_slStart; /* 開始位置(起点相対) */
  float    mv_slEnd;   /* 終了位置(起点相対) */

  /* 半径変更   */
  int      tm_scale;   /* 所要時間           */
  int      mv_scStart; /* 開始半径           */
  int      mv_scEnd;   /* 終了半径           */

  /* 縦引き伸ばし */
  int      tm_stretch; /* 所要時間           */
  int      mv_stStart; /* 開始高さ           */
  int      mv_stEnd;   /* 終了高さ           */

  /* UV アニメ */
  int      tm_uv;      /* 所要時間           */
  float    mv_uStart;  /* 開始 U             */
  float    mv_vStart;  /* 開始 V             */
  float    mv_uwStart; /* 開始 U 幅          */
  float    mv_vhStart; /* 開始 V 幅          */

  float    mv_uEnd;    /* 終了 U             */
  float    mv_vEnd;    /* 終了 V             */
  float    mv_uwEnd;   /* 終了 U 幅          */
  float    mv_vhEnd;   /* 終了 V 幅          */

  int      player_lookup:1;  /* プレイヤの動きに同期させるかどうか */

  int      plate:1;          /* 平板モードフラグ */

} CYLINDER_PARAM;

void * NewCylinderImageP(CYLINDER_PARAM * param);
void * NewCylinderImage(int name, int where);

#endif /* _cylinder_h_ */
