/*
  libdg頂点シェーダ用コンスタントレジスタテーブル/汎用定義

  2001/05/14 F.Miyauchi
  $Id: shader.h,v 1.5 2002/11/23 11:36:54 Yoshizawa1 Exp $
 */

/*
 * ワールド行列(EVM 補間用に最大8個必要)
 * ※evmで使用している行列インデックスは、
 *   0スタートを前提にしているので注意。
 */
#define CV_WORLD     0
#define CV_WORLD0    0
#define CV_WORLD1    4
#define CV_WORLD2    8
#define CV_WORLD3    12
#define CV_WORLD4    16
#define CV_WORLD5    20
#define CV_WORLD6    24
#define CV_WORLD7    28

/* DG_PRIM2用 */
#define CV_SCREEN	0		/* CV_WORLDの変わりに eye_inv * world を設定する */

/* 影シェーダ用(CV_WORLD1～7と共用) */
#define CV_MATSHADOW   4
#define CV_SHADOWALPHA 8

/* cp->eye_pers */
#define CV_EYEPERS   32
/* world * cp->eye_pers */
#define CV_WORLDEYEPERS   36	/* 多分これは使わない・・・ */
#define CV_PERS				36	/*  */
#define CV_OPTCMF_EYEPERS	36	/* 光学迷彩計算用 */

/* 光源計算用 */
#define CV_LIGHTVEC  40
#define CV_LIGHTCOL  44
#define CV_AMBIENT   47

/* 環境マップ計算用マトリクス */
#define CV_EYE_INV	48

/* Texture用 */
#define CV_TEX0_SCALE	60
#define CV_TEX0_OFFSET	61
#define CV_TEX1_SCALE	62
#define CV_TEX1_OFFSET	63
#define CV_TEX2_SCALE	64
#define CV_TEX2_OFFSET	65

/* 追加光源計算用 */
#define CV_ADDLIGHTVEC  48
#define CV_ADDLIGHTCOL  52
#define CV_ADDLIGHTPOS  56

/* 点光源計算用 */
#define CV_PLIGHTPOS    60
#define CV_PLIGHTCOL    64
#define CV_PLIGHTPARAM  68

/* フォグパラメータ
 * CV_FOG.x = Near
 * CV_FOG.y = Far
 */
#define CV_FOG          69

/* 定数
 *
 * CV_ZERO = 0.0f, 0.0f, 0.0f, 0.0f
 * CV_ONE  = 1.0f, 1.0f, 1.0f, 1.0f
 * CV_HALF = 0.5f, 0.5f, 0.5f, 0.5f
 * CV_SCALE = 1/4096, 1/255 1/16, 1
 */
#define CV_ZERO         80
#define CV_ONE          81
#define CV_HALF         82
#define CV_SCALE		83

/* その他汎用定数 */
#define CV_CONST0       84
#define CV_CONST1       85
#define CV_CONST2       86
#define CV_CONST3       87
#define CV_CONST4       88
#define CV_CONST5       89
#define CV_CONST6       90

/* COMDL用 */
#define CV_COMDL_COL    CV_CONST0

/* 定数レジスタの最大使用数 */
#define MAX_VERTEXSHADER_CONSTANT_NUM	(91)

/*-----------------------------------------------------------------------------*/

/* 入力データ */
#define V_POSITION v0
#define V_WEIGHT   v1
#define V_NORMAL   v2
#define V_DIFFUSE  v3

#define V_TEX0     v7
#define V_TEX1     v8
#define V_TEX2     v9
#define V_TEX3     v10

#define V_BLEND    v11
#define V_INDEX    v12

/* Windowsで追加 */
#define V_WEIGHT_WIN	v0

/*-----------------------------------------------------------------------------*/

/* レジスタ使用定義 */
#define R_POSITION		r0
#define R_NORMAL		r2
#define R_COL			r3
#define R_ADDCOL		r4
#define R_PLIGHT_COL	r5
#define R_SVEC			r6
#define R_TVEC			r7
#define R_HVEC			r8

#define R_TMP           r11
