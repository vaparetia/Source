/*
	cdc_face.c
	      無線 顔表示関連
	2000/07/24	Y.Kira
	$Id: cdc_face.c,v 1.1.1.3 2002/11/19 11:45:05 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "gameheader.h"
#include "codec.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"
#include "fmt_demo.h"
#include "dmapack.h"
#include "def_dma.h"
#define _cdc_face_c_
#include "cdc_load.h"
#include "cdc_face.h"
#include "codecmem.h"
#include "codeccrt.h"
#include "c_hsync.h"

#include "sprite_2d.h"
#include "3d_util.h"
#include "codecmtn.h"
#include "cdcobj.h"

#include "codec_config.h"
#include "../../kano/hair/hair_called.h"

#undef _DEBUG_
#define DEBUG

#ifdef _DEBUG_
#define DBG(args...)  printf(args)
#else
#define DBG(args...)
#endif /* _DEBUG_ */

#define MODEL_FLAG_0  (DG_EVMOBJ_INVISIBLE | DG_EVMOBJ_INVISIBLE3)
#define MODEL_FLAG_1  (DG_EVMOBJ_INVISIBLE | DG_EVMOBJ_INVISIBLE2)

#define F_PI    ((float)M_PI)
#define FOCUS_FUNC(t)  ((vu0_Sin(t) + vu0_Sin((t) * 2.0F)) / 2.5F)

/* プロトタイプ宣言は,狩野さんが提供しているヘッダを使ったほうが安全と判断。*/
/*
void *NewEvmHairModel_Wireless(int name,EVM_DEF *evm,DG_DEF *bound,
			       int sample_num,
			       OBJECT *target,int light_flag, int chanl, int draw_flag);
*/
void * NewFaceMotion(int name, DG_EVMOBJ *evmobj, int mouth[7], int eyes);
void   GM_SetLipPower(void *work, float power);
void   DG_SetCamera2(DG_CHANL *chanl,
		     FVECTOR *from, FVECTOR *to, float screen);
int    DG_QueueEvmObj(DG_EVMOBJ * evmobj);
void   DG_DequeueEvmObj(DG_EVMOBJ * evmobj);

void   UTL_StartMemToSpr(void *dst, void *src, int size);
void   UTL_EndMemToSpr(void);

typedef struct {
  FVECTOR root;
  FVECTOR joints[64];
  FVECTOR quat_buffer[4];
  FVECTOR vec;
  FMATRIX mat;
  FMATRIX root_mat;
  FMATRIX mats[64];
} ScrPadWork;


/*
 * 顔表示の前に描画する背景の板(Z 値を 1 以上にするために描画
 */
typedef struct {
  DG_DMATAG  dmatag;
  struct _base_gif {
    DG_GIFTAG  giftag;
    struct _base_data {
      DG_GSREG   zbuf0;
      DG_GSREG   test0;
      DG_GSREG   alpha;
      DG_GSREG   prim;
      DG_GSREG   rgbq0;
      DG_GSREG   xyz0;
      DG_GSREG   rgbq1;
      DG_GSREG   xyz1;
      DG_GSREG   zbuf1;
      DG_GSREG   test1;
    } data;
  } gif;
} BasePlate ALIGN16;

typedef struct {
  GV_ACT_EX    actor;

  FMATRIX   face_mat;
  FMATRIX   camera_work;  /* 顔カメラ位置マトリクス */
  FVECTOR   camera_pos;   /* カメラ位置             */
  FVECTOR   eye_target;   /* 注視点                 */
  FVECTOR   head_pos;     /* 頭部位置               */
  FMATRIX   light_matrix[2];
  FVECTOR   nutral[64];

  int name;

  OBJECT       face;       /* 顔オブジェクト                    */

  void       * face_works;  /* 顔アニメーションモジュールの Work 領域
			       へのポインタ */
  void       * effect_works;/* 画面効果モジュールの Work 領域へのポインタ   */
  void       * hsync_work;  /* 水平同期ずれ効果モジュールのWork 領域へのポインタ */


  int side;                 /* 二つの顔領域のうち、どちらに表示されているか */
  int disp;                 /* 表示/非表示                                  */
  int nxt_disp;
  int disp_fade;            /* 表示にフェードイン/アウトが伴うかどうか      */


  /* 背景 */
  BasePlate       * plate;
  DG_DMAPACK      * plate_dmapack;

  /* 上にかぶせるフィルタ */
  SPR_OBJ * over_filter;
  SPR_OBJ * mask_frame[2];  /* 画面縁のゴミを隠すフレーム */

  void    * handy_cam;


  /* 基本顔カメラ位置 */
  float     far, zoom;     /* カメラ距離とズーム       */
  float     heading, pan;  /* 横方向回転と、平行移動量 */
  float     pitch,   gain; /* 縦方向回転と、平行移動量 */

  /* カメラ操作のディレイタイム */
  float     spin_delay, zoom_delay;

  /* カメラのユーザ操作による補正値 */
  float     adjust_heading; /* heading 操作値 */
  float     adjust_pitch;   /* pitch 操作値   */

  /* 補正値と基本角度を加算した目標値 */
  float     target_heading; /* 目標 heading 値 */
  float     target_pitch;   /* 目標 pitch 値   */

  /* 計算結果の、そのフレームにおける表示用角度 */
  float     now_heading;    /* そのフレームでの heading 計算結果 */
  float     now_pitch;      /* そのフレームでの pitch 計算結果   */
  float     now_zoom;       /* そのフレームでの zoom 計算結果    */

  /* 操作リミッタ */
  float     heading_right;
  float     heading_left;
  float     pitch_upper;
  float     pitch_lower;
  float     zoom_limit;


  /* 話者情報: 話者 ID を保持 */
  int       speaker_id;

  /* 役者情報: 話者 ID + 1 の値を使用して、無線中モーションを読む */
  int       body_id;

  /* ダミー骨格用 kms 名称 */
  int       dummy_bone;


  /* 画面 ON/OFF 時のアクションのステップ */
  int       action_step;
  int       action_cnt;
  int       x_size, y_size;

  int       focus_name;    
  void    * nfocus_work;      /* 近景ぼかし Work */
  void    * ffocus_work;      /* 遠景ぼかし Work */

  /* ズーム処理用パラメタ */
  float     zoom_adjust;      /* ズーム値修正    */
  float     zoom_adjust_base; /* ズーム修正振幅  */
  float     zoom_goal;        /* ズーム変化曲線の終点 */
  int       zoom_wait;        /* ズーム調整を開始するまでの待ち時間 */
  int       zoom_time;        /* ズーム調整が終了するまでの時間     */
  int       zoom_cnt;         /* 経過時間                           */
  
  /* フォーカス処理用パラメタ */
  int       focus_time;      
  int       focus_cnt;
  int       focus_zone;

  /* ポーズを固定するためのフラグ */
  char      motion_fix;

  /* ストリーミングが始まっても、最初の数フレームはカメラを頭部に追従させる */
  int       mtn_camera_cnt;
  DEMO_MOTION * pre_motion;

  void    * hair_work;
  int       body_move:1;  /* 体にモーションを割り当てたことが
			     あるかどうかのフラグ */
} Work;

enum {
  STEP_CLOSE,     /* 閉じ状態                             */
  STEP_OPENING,   /* CRT の走査線が広がる最中             */
  STEP_FOCUS,     /* ズーム/フォーカシング                */
  STEP_OPENED,    /* 表示完了                             */
  STEP_CLOSING    /* 走査線が縮む                         */
};

#define  ACT_X_TIME   15.0F  /* x 方向に走査線が広がりきるまでの時間 */
#define  ACT_Y_DELAY  13.0F  /* y 方向が広がり始めるまでの遅延時間   */
#define  ACT_Y_TIME   80.0F /* y 方向が広がり始めてから広がりきるまでの時間 */

#define ZOOM_LIMIT_UNDER 9.0F  /* 最小ズーム値 */

static int face_flag[2];      /* チャネル 0,1 の表示状態保存用 */

/* 両サイドで現在有効になっている Work */
static Work    * now_work[2] = {NULL, NULL};

/* #define BASE 0 */
#define BASE 2


/* VGA スケールから、表示画面スケールへの変換マクロ */
#define xV2S(x)  (((x) * DRAW_WIDTH) / 640)
#define yV2S(y)  (((y) * DRAW_HEIGHT) / 480)

/* VGA スケールから、スプライト座標系スケールへの変換マクロ */
#define xV2SP(x)   ((x) * 512.0F / 640.0F)
#define yV2SP(y)   ((y) * 384.0F / 480.0F)

#define FACE_SMALL
#ifndef FACE_SMALL

/* 顔表示領域のサイズ */
#define FACE_WIDTH  xV2S(200)
#define FACE_HEIGHT yV2S(200)

/* 顔表示領域の高さ */
#define FACE_POS_Y  yV2S(CODEC_PARTS_LEVEL)


/* 外枠のサイズ */
#define OUTFRAME_WIDTH  xV2SP(180)
#define OUTFRAME_HEIGHT yV2SP(180)

#else

#define FACE_WIDTH      xV2S(140)
#define FACE_HEIGHT     yV2S(176)

/* 顔表示領域の高さ */
#define FACE_POS_Y  yV2S(CODEC_PARTS_LEVEL + 2)

#define OUTFRAME_WIDTH  xV2SP(180)
#define OUTFRAME_HEIGHT yV2SP(180)

#endif 

/* 左右の描画領域情報 */
/* 領域の位置は、画面中央の位置で指定する。 */

#define _LIKE_MGS1_

/*
 * 前作風顔配置
 */
static struct { int x, y, w, h; } env_list[] = {
  { xV2S(147), FACE_POS_Y, FACE_WIDTH, FACE_HEIGHT },
  { xV2S(492), FACE_POS_Y, FACE_WIDTH, FACE_HEIGHT }
};  

#define Qsize(_v)   (sizeof(_v) / sizeof(u_long128))


#ifdef DEBUG
static void disp_mat(FMATRIX * mat)
{
  int y;

  for(y = 0; y < 4; y++)
    printf("| %8.3f %8.3f %8.3f %8.3f |\n",
	   mat->m[0][y], mat->m[1][y], mat->m[2][y], mat->m[3][y]);
  printf("\n");
}

static void disp_vec(FVECTOR * vec)
{
  printf("(%8.3f, %8.3f, %8.3f, %8.3f)\n", vec->vx, vec->vy, vec->vz, vec->vw);
}
#endif /* DEBUG */

/*
 * ユーザカメラ操作
 */
#define ANALOG_MAX  (128 - ANALOG_MARGIN)

static void camera_control(Work * work)
{
  GV_PAD * pad = &GV_PadDataDirect[0];
  int dx, dy, zsw;   /* アナログスティックの傾斜値と、zoom on/off */
  int sx, sy;
  float fx, fy;
  float h_max, p_max, target_zoom;

  /* 基本的に、カメラ角操作値は 0.0F */
  work->adjust_heading = 0.0F;
  work->adjust_pitch   = 0.0F;

  /* スティック値の取得 */
  zsw = pad->status & ((!work->side) ? PAD_AL : PAD_AR);
  dx = ((!work->side) ? pad->left_dx : pad->right_dx) - 128;
  dy = ((!work->side) ? pad->left_dy : pad->right_dy) - 128;

  sx = (dx) ? (dx / abs(dx)) : 1;
  sy = (dy) ? (dy / abs(dy)) : 1;

  /* 傾斜値の絶対値が、マージン以上であれば操作として認識する */
  fx = (float)((abs(dx) >= ANALOG_MARGIN)
	       ? ((abs(dx) - ANALOG_MARGIN) * sx) : 0);
  fy = (float)((abs(dy) >= ANALOG_MARGIN)
	       ? ((abs(dy) - ANALOG_MARGIN) * sy) : 0);


#ifdef FACE_CAMERA_CHANGE

  {
    int status = GV_PadDataDirect[0].status & (PAD_L1|PAD_L2);
    int press =  GV_PadDataDirect[0].press & (PAD_R1|PAD_R2);
    /*
     * ボタンを押しながらの場合は,
     * カメラアングルではなくフレーミングの移動
     * (上下左右シフト)
     */

    switch(status)
      {
      case (PAD_L1|PAD_L2):    /* 同時押しで距離 */
	work->far -= fy / 10.0F;
	break;
      case PAD_L2:             /* L2 でズーム */
	work->zoom += fy / 200.0F;
	break;
      case PAD_L1:             /* L1 で位置    */
	work->pan -= fx / 10.0F;
	work->gain += fy / 10.0F;
	break;
      default:                 /* そのままで角度 */
	h_max = F_PI / 2.0F;
	p_max = F_PI / 4.0F;
	
	work->adjust_heading = h_max * fx / (float)ANALOG_MAX;
	work->adjust_pitch   = p_max * fy / (float)ANALOG_MAX;
	
	work->target_heading = work->heading + work->adjust_heading;
	work->target_pitch   = work->pitch   + work->adjust_pitch;
	break;
      }
    if(!status) 
    if((PAD_R2 | PAD_R1) == press)
      printf("facecamera %d %d %d %d %d %d %d\n",
	     work->side,
	     (int)(work->far * 1000.0F),
	     (int)(work->zoom * 1000.0F),
	     (int)(work->heading * 2048.0F / F_PI),
	     (int)(work->pan * 1000.0F),
	     (int)(work->pitch * 2048.0F / F_PI),
	     (int)(work->gain * 1000.0F));
  }
#else  
  /* 顔の回転(左右は最大±90°、上下は±45°) */
  h_max = (fx >= 0) ? work->heading_right : work->heading_left;
  p_max = (fy >= 0) ? work->pitch_upper : work->pitch_lower;

  work->adjust_heading = h_max * fx / (float)ANALOG_MAX;
  work->adjust_pitch   = p_max * fy / (float)ANALOG_MAX;

  work->target_heading = work->heading + work->adjust_heading;
  work->target_pitch   = work->pitch   + work->adjust_pitch;

#endif /* FACE_CAMERA_CHANGE */

  /*
   * ズーム
   */
  target_zoom = (!zsw) ? work->zoom : work->zoom_limit;

  /* 現在の回転角から、目標回転角までの角度を補間する */
  work->now_heading = work->now_heading +
    (work->target_heading - work->now_heading) / work->spin_delay;
  work->now_pitch = work->now_pitch +
    (work->target_pitch - work->now_pitch) / work->spin_delay;

#ifdef FACE_CAMERA_CHANGE
  /* カメラ角度がレバーを離しても固定されるようにする */
  work->heading = work->now_heading;
  work->pitch   = work->now_pitch;
#endif /* FACE_CAMERA_CHANGE */

  /* 現在の表示ズーム値から、目標ズーム値までの値を補間する */
  work->now_zoom = work->now_zoom +
    (target_zoom - work->now_zoom) / work->zoom_delay;



  DBG("ZOOM: now = %8.3f  target = %8.3f  delay: %8.3f\n",
	 work->now_zoom, target_zoom, work->zoom_delay);

  /* フォーカス処理(他のソフトフォーカス処理をしていないときのみ) */
  if(work->focus_cnt < 0)
    {
      if((int)(target_zoom - work->now_zoom))
	{
	  float rate;
	  soft_focus_switch(work, 1);
	  rate = (work->now_zoom - work->zoom) /
	    (work->zoom_limit - work->zoom);
	  if(rate < 0.0F) rate = -rate;
	  if(rate > 1.0F) rate = 1.0F;
	  work->focus_zone = (int)(work->far + FOCUS_FUNC(rate) * work->far);
	  set_focus_area(work, work->focus_zone);
	}
      else
	soft_focus_switch(work, 0);
    }
}

/*
 * 顔カメラ位置パラメータから、カメラ位置変更マトリクスを作成
 */
static void camera_position(Work * work)
{
  FMATRIX mat;
  FVECTOR trans;
  FVECTOR rot;
  int x, y;

  for(y = 0; y < 4; y++)
    for(x = 0; x < 4; x++)
      mat.m[x][y] = (float)(x == y);

  /* 平行移動 */
  trans.vx = work->pan;
  trans.vy = work->gain;
  trans.vz = 0.0F;
  trans.vw = 1.0F;

  /* 回転 */
  rot.vx = work->now_pitch;
  rot.vy = work->now_heading;
  rot.vz = 0.0F;
  rot.vw = 1.0F;

  _sceVu0RotMatrix(&mat, &mat, &rot);
  _sceVu0TransMatrix(&work->camera_work, &mat, &trans);
 
  /* disp_mat(&work->camera_work); */

}

static void camera_move(Work * work)
{
  /*
   * 基準となるカメラ位置と注視点を作成
   */
  /* カメラ位置 */
  work->camera_pos.vx = 0.0F;
  work->camera_pos.vy = 0.0F;
  work->camera_pos.vz = work->far;
  work->camera_pos.vw = 1.0F;

  /* 注視点 */
  work->eye_target.vx = 0.0F;
  work->eye_target.vy = 0.0F;
  work->eye_target.vz = 0.0F;
  work->eye_target.vw = 1.0F;

  /*
   * それぞれのベクトルに変換行列をかけて、
   * 基準注視点を原点とした値を計算する
   */
  
  _sceVu0ApplyMatrix(&work->camera_pos, &work->camera_work, &work->camera_pos);
  _sceVu0ApplyMatrix(&work->eye_target, &work->camera_work, &work->eye_target);

  work->eye_target.vw = 0.0F;
  work->camera_pos.vw = 0.0F;
}


/*
 * 無線顔カメラ位置設定
 */
int cdcFaceCamera(int side,
		  float far, float zoom,
		  float heading, float pan,
		  float pitch,   float gain)
{
  Work * work = now_work[side];

  if(NULL == work) return -1;

  work->heading  = heading;
  work->pan      = pan;
  work->pitch    = pitch;
  work->gain     = gain;
  work->far      = far;
  work->zoom     = zoom;
 
  work->now_heading = heading;
  work->now_pitch   = pitch;
  work->now_zoom    = zoom;

  return 0;
}

/*
 * 無線顔カメラ操作リミッタ設定
 */
int cdcFaceLimit(int side, float zoom,
		 float right, float left, float upper, float lower)
{
  Work * work = now_work[side];

  ASSERT(NULL != work);

  work->zoom_limit    = zoom;
  work->heading_right = right;
  work->heading_left  = left;
  work->pitch_upper   = upper;
  work->pitch_lower   = lower;
  return 0;
}

/*
 * 無線顔カメラ操作ディレイ値設定
 */
int cdcFaceControlSpeed(int side, int spin_delay, int zoom_delay)
{
  Work * work = now_work[side];

  ASSERT(NULL != work);

  work->spin_delay = (float)spin_delay / (float)TIME_BASE;
  work->zoom_delay = (float)zoom_delay / (float)TIME_BASE;
  return 0;
}


/*
 * 背景板設定
 */
static int setup_bg_plate(Work * work)
{
  static BasePlate def_plate = {
    dmatag:
    {
      qwc: DMATAG_SET_QWC(DMATAG_ID_RET, Qsize(struct _base_gif)),
      vifcode:
      {
	SCE_VIF1_SET_NOP(0),
	SCE_VIF1_SET_DIRECT(Qsize(struct _base_gif), 0)
      }
    },
    gif:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(Qsize(struct _base_data), 1, 0, 0, 0, 1),
	regs: GS_REGS_AD
      },
      data:
      {
	zbuf0: { reg: SCE_GS_ZBUF_1 },
	test0:
	{
	  reg: SCE_GS_TEST_1,
	  data: SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1)
	},
	alpha:
	{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(0, 2, 2, 2, 128)
	},
	prim:
	{
	  reg: SCE_GS_PRIM,
	  data: SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 1, 0, 0)
	},
	rgbq0:
	{
	  reg: SCE_GS_RGBAQ,
	  data: SCE_GS_SET_RGBAQ(FACEBG_R, FACEBG_G, FACEBG_B, 128, 0)
	},
	xyz0: { reg: SCE_GS_XYZ2 },
	rgbq1:
	{
	  reg: SCE_GS_RGBAQ,
	  data: SCE_GS_SET_RGBAQ(FACEBG_R, FACEBG_G, FACEBG_B, 128, 0)
	},
	xyz1: { reg: SCE_GS_XYZ2 },
	test1: { reg: SCE_GS_TEST_1 },
	zbuf1: { reg: SCE_GS_ZBUF_1 }
      }
    }
  };
  int i, w, h, flags;
  u_long zbuf, test;
  DG_DMAPACK * dmapack;


  DBG("Setup Face BG!!\n");
  work->plate = codecMalloc(sizeof(BasePlate) * 2);

  ASSERT(work->plate != NULL);

  flags =( DG_DMAPACK_NORMAL     | DG_DMAPACK_PRIVILEGE  |
	   DG_DMAPACK_INVISIBLE0 | DG_DMAPACK_INVISIBLE1 |
	   DG_DMAPACK_INVISIBLEMENU );

  flags |= (work->side) ? DG_DMAPACK_INVISIBLE2 : DG_DMAPACK_INVISIBLE3;

  dmapack = DG_MakeDmapack(flags, DG_DMAPACK_PHASE_FIRST);
  ASSERT(dmapack != NULL);
  w = FACE_WIDTH;
  h = FACE_HEIGHT;

  for(i = 0; i < 2; i++)
    {
      printf("w = %d, h = %d\n", w, h);

      work->plate[i] = def_plate;
      work->plate[i].gif.data.xyz0.data = SCE_GS_SET_XYZ((2048 - w / 2) * 16,
							 (2048 - h / 2) * 16,
							 1);
      work->plate[i].gif.data.xyz1.data = SCE_GS_SET_XYZ((2048 + w / 2) * 16,
							 (2048 + h / 2) * 16,
							 1);

      zbuf = *((u_long *)&DG_Chanls[work->side + 2].draw_env[i].datas.zbuf1);
      test = *((u_long *)&DG_Chanls[work->side + 2].draw_env[i].datas.test1);


      printf("zbuf = %016lx   test = %016lx\n", zbuf, test);
      work->plate[i].gif.data.zbuf0.data = zbuf & ~I64(0x100000000);

      /* 描画環境復元設定 */
      work->plate[i].gif.data.zbuf1.data = zbuf;
      work->plate[i].gif.data.test1.data = test;
      
      dmapack->packet[i] = &work->plate[i];
    }
  dmapack->flag = flags;
  work->plate_dmapack = dmapack;
  DG_QueueDmapack(dmapack);
  return 0;
}

/*
 * フォーカスのぼかし ON/OFF
 */
static void soft_focus_switch(Work * work, int sw)
{
#ifdef EFFECT_FOCUS
  nfocusSwitch(work->nfocus_work, sw);
  ffocusSwitch(work->ffocus_work, sw);
#endif /* EFFECT_FOCUS */
}

/*
 * 被写界深度設定
 */
static void set_focus_area(Work * work, int zone)
{
  int near, far;

  near = zone - 20;
  far = zone + 20;
#ifdef EFFECT_FOCUS
  nfocusSetFocus(work->nfocus_work, 0, 0, near - 100, near);
  ffocusSetFocus(work->ffocus_work, 0, 0, far, far + 100);
#endif /* EFFECT_FOCUS */
}

static void init_focus(Work * work)
{
  float rate;

  work->focus_time = (int)((float)work->zoom_time / 0.8F);
  work->focus_cnt = 0;
  if(work->focus_time)
    {
      rate = (F_PI - 1.0F) * (float)work->focus_cnt / (float)work->focus_time + 1.0F;
      work->focus_zone = (int)(work->far + FOCUS_FUNC(rate) * work->far);
      printf("focus_zone = %d\n", work->focus_zone);
      set_focus_area(work, work->focus_zone);
    }
}

static int focus_proc(Work *work)
{
  float rate;

  if(work->focus_cnt < 0) return 0;
  if(work->focus_cnt >= work->focus_time)
    {
      soft_focus_switch(work, 0);
      work->focus_cnt = -1;
      return 0;
    }
  work->focus_cnt += TIME_BASE;
  rate = (F_PI - 1.0F) * (float)work->focus_cnt / (float)work->focus_time + 1.0F;
  work->focus_zone = (int)(work->far + FOCUS_FUNC(rate) * work->far);
  set_focus_area(work, work->focus_zone);
  return 1;
}


#define ZOOM_TIME   200
#define ZOOM_WAIT   0
#define ZOOM_START  1.0F
/*
 * ズーム調整初期化
 */
static void zoom_init(Work * work)
{
  int a;
  float rate;

  soft_focus_switch(work, 1);
  work->zoom_adjust_base = (float)((rand() >> 16) % 3000) / -1000.0F;
  a = (rand() >> 16) & 1;
  work->zoom_goal = (a) ? F_PI : (F_PI * 4 / 3);
  work->zoom_time = ZOOM_TIME + ((rand() >> 16) % 20) - 10;
  work->zoom_wait = 0; /* ZOOM_WAIT + ((rand() >> 16) % 10) - 5; */
  
  work->zoom_cnt = 0;

  rate = ZOOM_START;
  rate = (vu0_Sin(rate * 1.0F) + vu0_Sin(rate * 2.0F)) / 1.8;
  work->zoom_adjust = work->zoom_adjust_base * rate;
  /* work->zoom_adjust = 0.0F; */
  init_focus(work);
}

/*
 * ズーム調整プロセス
 */
static int zoom_proc(Work * work)
{
  float rate, t;

  if(work->zoom_wait > 0)
    {
      work->zoom_wait -= TIME_BASE;
      return 1;
    }

  if(work->zoom_cnt >= work->zoom_time) return 0;

  t = (float)work->zoom_cnt / (float)work->zoom_time;

  rate = (work->zoom_goal - ZOOM_START) * t + ZOOM_START;

  /* rate = work->zoom_goal * t; */
  
  rate = (vu0_Sin(rate) + vu0_Sin(rate * 2.0F)) / 2.5F * (1.0F - t);

  work->zoom_adjust = work->zoom_adjust_base * rate;

  work->zoom_cnt += TIME_BASE;

  return 1;
}

/*
 * 髪の毛の設定(ライデン、ローズ用)
 */
void codecSetupHair(int side,
		    void * model, int sample_num,
		    void * bound, int hit_param, int ambient)
{
  FVECTOR quat;
  Work * work = now_work[side];
  void * hair_work;

  ASSERT(NULL != work);
  /*
    if(NULL == bound)
    bound = GV_GetCache(GV_CacheID(DUMMY_BONE, 'k'));
  */
#ifdef DEBUG
  DBG("model = %p\n", model);
  DBG("bound = %p\n", bound);
  DBG("sample_num = %d\n", sample_num);
  DBG("ambient = %d\n", ambient);
  DBG("chanl = %d\n", BASE + side);
#endif /* DEBUG */


  /* 髪の毛設定 */

#ifndef EFFECT_HAIR
  return;
#endif /* EFFECT_HAIR */

  hair_work = NewEvmHairModel_Wireless(0,               /* name */
				       model,           /* evm  */
				       bound,           /* bound */
				       sample_num,      /* sample_num */
				       &(work->face),   /* target */
				       ambient,         /* light_flag */
				       BASE + side,     /* chanl */
				       DELAY_DRAW);     /* delay flag */

  ASSERT(hair_work != NULL);
  DBG("Hair Setup!!\n");
  GV_SetActorChild(work, hair_work);
  work->hair_work = hair_work;
}

/*
 * 顔表示初期化
 */
void codecSetupScreen(void)
{
  int i;

  /* チャネル 0, 1 の表示状態保存および非表示化 */
  for(i = 0; i < 2; i++)
    face_flag[i] = DG_Chanls[i].flag;   /* 表示状態の保存 */ 

  /* チャネル 0 は非表示。 */
  DG_Chanls[0].flag = 0;
  DG_Chanls[0].bg_clear_flag = 1;  /* 毎フレーム背景を消去 */


  /* チャネル 1 は非表示 */
  DG_Chanls[1].flag = 0;

  /* チャネル 2, 3 を初期化するが、まだ表示状態にはしない */
  for(i = 0; i < 2; i++)
    {
      /* 表示領域の設定 */
      DG_SetDrawEnv(&DG_Chanls[i + BASE],
		    (env_list[i].x - DRAW_WIDTH / 2),
		    (env_list[i].y - DRAW_HEIGHT / 2),
		    env_list[i].w, env_list[i].h);
      
      /* カメラの設定 */
      BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
      /* DG_SetCamera2(&DG_Chanls[i + BASE], &eye[i], &center[i], def_zoom[i]); */

      /* 表示状態に移行 */
      DG_Chanls[i + BASE].flag = 0;
      DG_Chanls[i + BASE].bg_clear_flag = 1;  /* 毎フレーム背景を消去 */
    }
}

/*
 * 表示状態の復帰
 */
void codecRecoveryScreen(void)
{
  int i;

  /* 2, 3 を非表示に戻す */
  for(i = 2; i < 4; i++)
    DG_Chanls[i].flag = 0;

  /* 表示状態を復帰する */
  for(i = 0; i < 2; i++)
    DG_Chanls[i].flag = face_flag[i];
}

#ifdef FACE_DEFAULT_MOTION
/*
 * デフォルトモーションの設定
 */
static int setup_default_motion(Work * work, int mar, int id)
{
  DBG("-d-\n");
  DBG("setup_default_motion()\n");

  CDC_ConfigObjectMotion(&(work->face), 1, mar, 0);
  GM_ConfigObjectAction(&(work->face), 0, id, 0, 0, 0);

  return 0;
}

void codecSetDefaultMotion(int side, int motion)
{
  DBG("-b-\n");
  if(now_work[side] == NULL) return;
  DBG("-c-\n");
  setup_default_motion(now_work[side], motion, 0);
}

/*
 * Evm モデルの world を DG_OBJS の各 objs に反映する
 */
static void syncro_obj(Work * work)
{
  OBJECT * object = &work->face;
  DG_EVMOBJ * evmobj = object->evmobj;
  DG_OBJS * objs = object->objs;
  DG_OBJ *obj=objs->objs;
  EVM_SKEL * skel;
  FMATRIX * mats;
  FVECTOR vec;
  int i;

  skel = evmobj->def->skeleton ;
  mats = evmobj->matrix[ evmobj->use_buffer ];
  vec.vw = 1.0F;

  for(i=objs->n_models;i>0;i--){
    vec.vx = skel->rt_tx ;
    vec.vy = skel->rt_ty ;
    vec.vz = skel->rt_tz ;
    fpu_CopyMatrix(&(obj->world),mats);
    _sceVu0ApplyMatrix( (FVECTOR*)&(obj->world.m[3][0]), mats, &vec );
    skel++;
    mats++;
    obj++;
  }
  
  // body->objs->world = evmobj->world;
}


/*
 * デフォルトモーション再生
 */
static void play_default_motion(Work * work)
{
  SVECTOR rot;
  FVECTOR mov;

  /* デフォルトモーションが設定されていない場合は何もしない */
  GM_ActMotion(&(work->face));
  // 原点に設定
  mov = (FVECTOR){0.0F, 0.0F, 0.0F, 0.0F};
  rot = (SVECTOR){0, 0, 0, 0};
  DG_SetPos2(&mov, &rot);
  GM_ActObject2(&(work->face));
  /* EVM のマトリクスを OBJ に書き戻す */
  syncro_obj(work);
}

/*
 * デフォルトモーション開放
 */
static void destroy_default_motion(Work * work)
{
}
#endif /* FACE_DEFAULT_MOTION */


static void set_joints(DG_EVMOBJ * evmobj,
		       int start, int num, FVECTOR * vecarr)
{
  int i;
  ScrPadWork * scrpad = (ScrPadWork *)SCRPAD_ADDR;
  FMATRIX * skel_mats;
  FVECTOR   vec;
  FMATRIX * mats = scrpad->mats;
  EVM_SKEL * skel;
  FVECTOR * rot;
  FVECTOR * trans;

  skel_mats = evmobj->matrix[ evmobj->use_buffer ];
  skel = evmobj->def->skeleton + start;
  vec.vw = 1.0F;
  for(i = 0; i < num; i++)
    {
      FMATRIX * parent;
      if(i + start)
	{
	  /* クォータニオンとトランスレーションの取得 */
	  rot   = &vecarr[i * 2];
	  trans = &vecarr[i * 2 + 1];
      
	  MT_QuatToMat(mats, rot);
	  mats->m[3][0] = skel->rt_tx + trans->vx;
	  mats->m[3][1] = skel->rt_ty + trans->vy;
	  mats->m[3][2] = skel->rt_tz + trans->vz;
	  
	  parent = &skel_mats[ skel->parent ];
	  _sceVu0MulMatrix(mats, parent, mats);
	  vec.vx = -skel->rt_tx;
	  vec.vy = -skel->rt_ty;
	  vec.vz = -skel->rt_tz;
	  _sceVu0ApplyMatrix(&mats->m[3][0], mats, &vec);
	  skel_mats[i + start] = *mats;
	}
      skel++;
      mats++;
    }
}


static void setup_camera(Work * work, int move)
{
  /* 現在の姿勢における頭の位置を得る */
  if(move)
    get_joints_vector(&work->head_pos, work->face.evmobj, HUMAN21_ATAMA);

  /* 計算済のカメラ位置と注視点を、頭の位置まで平行移動する */
  work->eye_target.vx += work->head_pos.vx;
  work->eye_target.vy += work->head_pos.vy;
  work->eye_target.vz += work->head_pos.vz;
  work->eye_target.vw *= work->head_pos.vw;
  
  work->camera_pos.vx += work->head_pos.vx;
  work->camera_pos.vy += work->head_pos.vy;
  work->camera_pos.vz += work->head_pos.vz;
  work->camera_pos.vw *= work->head_pos.vw;


  BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
  DG_SetCamera2(&DG_Chanls[work->side + BASE],
		&work->camera_pos, &work->eye_target,
		work->now_zoom + work->zoom_adjust);

#if 0
  {
    int i;

    DBG("HEAD: ");
    disp_vec(&work->head_pos);

    DBG("camera <%d>\n", work->side);
    DBG("camera: ");
    disp_vec(&work->camera_pos);
    DBG("target: ");
    disp_vec(&work->eye_target);
    disp_mat(&DG_Chanls[work->side + BASE].eye_pers);
    DBG("\n");
  }
#endif
  


}

static void set_filter_size(Work * work, float w_rate, float h_rate)
{
  /* 上にかけるフィルタの位置、サイズを調整 */
  int side;
  int chanl;
  int width, height;
  SPR_POS pos;
  SPR_RECT rect;
  float w, h;
  float fw, fh;
  int i;

  side = work->side;
  chanl = side + BASE;

  width  = (int)((float)env_list[side].w * w_rate);
  height = (int)((float)env_list[side].h * h_rate);

  w = width * 512.0F / DRAW_WIDTH;
  h = height * 384.0F / DRAW_HEIGHT;
  pos.x = (env_list[side].x - width / 2) * 512.0F / DRAW_WIDTH;
  pos.y = (env_list[side].y - height / 2) * 384.0F / DRAW_HEIGHT;
  SPR_SetPosSprite(work->over_filter, &pos);
  SPR_SetSizeSprite(work->over_filter, w, h);
  
  fw = (float)DRAW_WIDTH / 512.0F;
  fh = (float)DRAW_HEIGHT / 384.0F;
  
  for(i = 0; i < 2; i++)
    {
      rect.begin.x = 0.0F + (float)i * fw;
      rect.begin.y = 0.0F + (float)i * fh;
      rect.end.x = w - (float)(1 + i) * fw;
      rect.end.y = h - (float)(1 + i) * fh;
      SPR_SetPosBox(work->mask_frame[i], &rect);
      SPR_SetColorBox(work->mask_frame[i], 40, 110, 90, 128);
      work->mask_frame[i]->head.flags |= SPR_FLAG_PRIV;
      SPR_SHOW(work->mask_frame[i]);
    }

#ifndef NO_GREEN_FILTER
  SPR_SHOW(work->over_filter);
#endif /* NO_GREEN_FILTER */
}

static void set_disp_env(Work * work, float w_rate, float h_rate)
{
  int side;
  int chanl;
  int width, height;

  side = work->side;
  chanl = side + BASE;

  width  = (int)((float)env_list[side].w * w_rate);
  height = (int)((float)env_list[side].h * h_rate);


  if(width == 0)
    {
      DG_Chanls[chanl].flag = 0;
      SPR_HIDE(work->over_filter);
      return;
    }
  if(height == 0) height = 1;  /* 表示される際は、最低でも 1ラスタは表示する */

  /* 表示領域の設定 */
  DG_SetDrawEnv(&DG_Chanls[chanl],
		(env_list[side].x - DRAW_WIDTH / 2),
		(env_list[side].y - DRAW_HEIGHT / 2),
		width, height);

  /* カメラの設定 */
  BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
  DG_SetCamera2(&DG_Chanls[chanl],
		&work->camera_pos, &work->eye_target, work->now_zoom);
  
  /* 表示状態に移行 */
  DG_Chanls[chanl].bg_clear_flag = 1;  /* 毎フレーム背景を消去 */
  DG_Chanls[chanl].flag = 1;
}

static void disp_sequence(Work * work)
{
  int side;

  side = work->side;
  switch(work->action_step)
    {
    case STEP_CLOSE:    /* 閉じ状態(非表示)        */
      set_disp_env(work, 0.0F, 0.0F);
      work->action_cnt = 0;
      work->focus_cnt = -1;
      break;
    case STEP_OPENING:  /* 開いている途中          */
      {
	float x_t_rate;  /* 時間軸レート(x) */
	float y_t_rate;  /* 時間軸レート(y) */
	float x, y, w_rate, h_rate;
	int cnt;

	/* フィルタのサイズを設定 */
	x_t_rate = (float)work->action_cnt / ACT_X_TIME;
	y_t_rate = ((float)work->action_cnt - ACT_Y_DELAY) / ACT_Y_TIME;

	if(x_t_rate > 1.0F) x_t_rate = 1.0F;
	if(x_t_rate < 0.0F) x_t_rate = 0.0F;

	if(y_t_rate > 1.0F) y_t_rate = 1.0F;
	if(y_t_rate < 0.0F) y_t_rate = 0.0F;

	x = x_t_rate;
	y = y_t_rate;

	w_rate = x * x * x * x;
	h_rate = y * y * y * y;

	set_filter_size(work, w_rate, h_rate);

	/* 表示領域のサイズを設定 */

	/* スプライトの表示が1フレーム遅れるため、
	   直前のフレームのサイズに合わせる。 */
	cnt = work->action_cnt - TIME_BASE;
	x_t_rate = (float)cnt / ACT_X_TIME;
	y_t_rate = ((float)cnt - ACT_Y_DELAY) / ACT_Y_TIME;

	if(x_t_rate > 1.0F) x_t_rate = 1.0F;
	if(x_t_rate < 0.0F) x_t_rate = 0.0F;

	if(y_t_rate > 1.0F) y_t_rate = 1.0F;
	if(y_t_rate < 0.0F) y_t_rate = 0.0F;

	x = x_t_rate;
	y = y_t_rate;

	w_rate = x * x * x * x;
	h_rate = y * y * y * y;

	set_disp_env(work, w_rate, h_rate);
	if((w_rate == 1.0F) && (h_rate == 1.0F))
	  work->action_step = STEP_FOCUS;
	work->action_cnt += TIME_BASE;
      }
      break;
    case STEP_FOCUS:    /* ズーム/フォーカシング中 */
      set_filter_size(work, 1.0F, 1.0F);
      set_disp_env(work, 1.0F, 1.0F);
#ifdef EFFECT_FOCUS
      if(!zoom_proc(work)) work->action_step = STEP_OPENED;
#else  /* EFFECT_FOCUS */
      work->action_step = STEP_OPENED;
#endif /* EFFECT_FOCUS */
      break;
    case STEP_OPENED:   /* 表示完了                */
      set_filter_size(work, 1.0F, 1.0F);
      set_disp_env(work, 1.0F, 1.0F);
      work->action_cnt = (int)(ACT_Y_DELAY + ACT_Y_TIME);
      break;
    case STEP_CLOSING:  /* 閉じ途中                */
      {
	float x_t_rate;  /* 時間軸レート(x) */
	float y_t_rate;  /* 時間軸レート(y) */
	float x, y, w_rate, h_rate;
	int cnt;

	x_t_rate = (float)work->action_cnt / ACT_X_TIME;
	y_t_rate = ((float)work->action_cnt - ACT_Y_DELAY) / ACT_Y_TIME;

	if(x_t_rate > 1.0F) x_t_rate = 1.0F;
	if(x_t_rate < 0.0F) x_t_rate = 0.0F;

	if(y_t_rate > 1.0F) y_t_rate = 1.0F;
	if(y_t_rate < 0.0F) y_t_rate = 0.0F;

	x = x_t_rate;
	y = y_t_rate;

	w_rate = x * x * x * x;
	h_rate = y * y * y * y;

	/* フィルタのサイズを設定 */
	set_filter_size(work, w_rate, h_rate);

	/* 1フレームの差を埋める */
	cnt = work->action_cnt + TIME_BASE;
	x_t_rate = (float)cnt / ACT_X_TIME;
	y_t_rate = ((float)cnt - ACT_Y_DELAY) / ACT_Y_TIME;

	if(x_t_rate > 1.0F) x_t_rate = 1.0F;
	if(x_t_rate < 0.0F) x_t_rate = 0.0F;

	if(y_t_rate > 1.0F) y_t_rate = 1.0F;
	if(y_t_rate < 0.0F) y_t_rate = 0.0F;

	x = x_t_rate;
	y = y_t_rate;

	w_rate = x * x * x * x;
	h_rate = y * y * y * y;

	/* 表示環境のサイズを設定 */
	set_disp_env(work, w_rate, h_rate);
	if((w_rate == 0.0F) && (h_rate == 0.0F))
	  work->action_step = STEP_CLOSE;
	work->action_cnt -= TIME_BASE;
      }
      break;
    }
}

void codecCloseFace(Work * work)
{
  init_focus(work);
  work->action_step = STEP_CLOSING;
}

int codecFaceIsClosed(Work * work)
{
  if(work->action_step == STEP_CLOSE) return 1;
  return 0;
}



/*****************************************************************************/
/*****************************************************************************/
static void Act(Work * work)
{
  int move = 1;

  /* 表示 / 非表示の切替えを行う */
  if(work->disp != work->nxt_disp)
    {
      work->disp = work->nxt_disp;   /* 表示状態を最新の状態に更新 */
      if(!work->disp)
	{
	  printf("BackFrame Hide!!\n");
	  work->action_step = (!work->disp_fade) ? STEP_CLOSING : STEP_CLOSE;
	  // SPR_HIDE(work->over_filter);
	}
      else
	{
	  printf("BackFrame Show!!\n");
	  work->action_step = (!work->disp_fade) ? STEP_OPENING : STEP_OPENED;
	  zoom_init(work);
	  if(work->disp_fade)
	    {
	      work->zoom_adjust_base = work->zoom_adjust = 0.0F;
	      work->focus_cnt = -1;
	      soft_focus_switch(work, 0);
	      work->zoom_cnt = work->zoom_time = 0;
	    }
	  // SPR_SHOW(work->over_filter);
	}
      DBG("Side %d: disp flag change to %d.\n", work->side, work->disp);
    }

  DBG("focus_proc()\n");
  focus_proc(work);  /* フォーカス処理 */
  /*
   * 現在の表示シーケンスに従い、描画環境を変更する
   */
  DBG("disp_sequence()\n");
  disp_sequence(work);

  /*
    evmobj->light = GM_PlayerBody->objs->light;
    evmobj->world = GM_PlayerBody->objs->world;
  */
  /* evmobj->world.m[3][2] += 1500.0f; */

  /* モーションのストリーミング再生 */
  {
    DEMO_MOTION * mtn;
    int start;

#ifndef NO_BODY_MOTION
    DBG("body motion\n");
    /* 顔モーションを割り当てる前に、身体モーションを割り当てる */
    if(NULL != (mtn = GM_StreamGetMotion(work->body_id)))
      {
	/* モーションがある場合はそのモーションを再生する */
	set_joints(work->face.evmobj, 0, mtn->n_joints, mtn->motion);
	/*	if(work->mtn_camera_cnt < 0)
	  work->mtn_camera_cnt++;
	  else
	*/
	/* ストリーミングモーション再生中は注視点を動かさない */
	if(work->body_move) move = 0;
	work->body_move = 1;
	work->pre_motion = mtn;
      }
#ifdef FACE_DEFAULT_MOTION
    else
      {
	/* モーションが無かった場合は、デフォルトモーションの再生を行う */
	if(work->pre_motion != NULL)
	  {
	    /* 直前のフレームまでストリーミングモーションの
	       再生を行っていた場合、デフォルトモーションから
	       再度2フレーム分を取り直す。 */

	    /* NULL ではないことが重要であるためポインタの値は特に意味は無い */
	    work->pre_motion = work;
	    work->motion_fix = 0;
	  }
	play_default_motion(work);
	work->body_move = 1;
      }
#endif  /* FACE_DEFAULT_MOTION */
#endif  /* NO_BODY_MOTION */

#ifndef NO_FACE_MOTION
    /* 現在のフレームの顔モーションを得て、
       身体モーションを適用しおわった直後の EVM モデルに上書きする */
    if(work->face.evmobj->n_skeleton >= 53)
      if(NULL != (mtn = GM_StreamGetMotion(work->speaker_id)))
	{
	  /* 全関節の値を設定する */
	  start = (mtn->n_joints == 32) ? 21 : 0;
	  set_joints(work->face.evmobj, start, mtn->n_joints, mtn->motion);
	}
      else
	{
	  /* モーションが無い場合は、顔の全関節をニュートラルにする。
	   * ※音声をキャンセルした際に、口が開きっぱなしになるのを防ぐため。
	   *   ただし、53関節モデルに限定する。
	   *   忍者のような口を動かさない 21関節モデルでは何もしない。*/
	  set_joints(work->face.evmobj, 21, 32, &work->nutral);
	}
#endif  /* NO_FACE_MOTION */
  }

  /* カメラ操作による補正パラメータ生成 */
  camera_control(work);

  /* カメラ位置操作マトリクスを作成する */
  camera_position(work);  /* カメラ位置回転行列を作成 */

  /* カメラ位置基準ベクトルと注視点ベクトルを作成し、
     回転行列をかけて実際のカメラ位置と注視点に変換する */
  camera_move(work);
  setup_camera(work, move);

#ifdef EFFECT_HANDYCAM
  {
    FMATRIX * mat;
    
    mat = hcGetMatrix(work->handy_cam);
    
    _sceVu0MulMatrix(&DG_Chanls[work->side + BASE].eye_pers,
		     &DG_Chanls[work->side + BASE].eye_pers, mat);
  }
#endif /* EFFECT_HANDYCAM */

}

static void Die(Work * work)
{
  int i;

  if(now_work[work->side] == work) now_work[work->side] = NULL;
  /*
    if(work->effect_works != NULL) GV_DestroyActor(work->effect_works);
    if(work->hsync_work != NULL) GV_DestroyActor(work->hsync_work);
  */
#ifdef FACE_DEFAULT_MOTION
  destroy_default_motion(work);
#endif /* FACE_DEFAULT_MOTION */
  if(work->over_filter != NULL) SPR_Destroy_2D_Object(work->over_filter);
  if(work->plate_dmapack != NULL)
    {
      DG_DequeueDmapack(work->plate_dmapack);
      DG_FreeDmapack(work->plate_dmapack);
      DBG("call codecFree()\n");
      codecFree(work->plate);
    }

  CDC_FreeObject(&(work->face));
  for(i = 0; i < 2; i++) DG_Chanls[i + BASE].flag = 0;
  DBG("Die.................\n");
}

/* 指定された表示サイドの主光源ベクトルを設定 */
int codecSetFaceLightVector(int side, float x, float y, float z)
{
  Work * work = now_work[side];
  FVECTOR vec;

  ASSERT(NULL != work);
  vec.vx = x;
  vec.vy = y;
  vec.vz = z;
  _sceVu0Normalize( &vec, &vec);
  work->light_matrix[0].m[0][0] = vec.vx;
  work->light_matrix[0].m[1][0] = vec.vy;
  work->light_matrix[0].m[2][0] = vec.vz;

  return 0;
}

/* 指定された表示サイドの主光源色を設定 */
int codecSetFaceLightColor(int side, int r, int g, int b)
{
  Work * work = now_work[side];

  ASSERT(NULL != work);

  work->light_matrix[1].m[0][0] = (float)r;
  work->light_matrix[1].m[0][1] = (float)g;
  work->light_matrix[1].m[0][2] = (float)b;
  return 0;
}

/* 指定された表示サイドの Ambient を設定 */
int codecSetFaceAmbient(int side, int r, int g, int b)
{
  Work * work = now_work[side];

  ASSERT(NULL != work);

  work->light_matrix[1].m[3][0] = (float)r;
  work->light_matrix[1].m[3][1] = (float)g;
  work->light_matrix[1].m[3][2] = (float)b;
  return 0;
}

void codecFaceDisp(int side, int sw, int fade)
{
  if(NULL == now_work[side]) return;
  now_work[side]->nxt_disp = sw;
  now_work[side]->disp_fade = fade;
}

/*
 * 顔表示のライティングを変更する
 */
static int face_lighting(Work * work)
{
  int x, y;

  /* 光源マトリクスの初期化 */
  for(y = 0; y < 4; y++)
    for(x = 0; x < 4; x++)
      {
	work->light_matrix[0].m[x][y] = 0.0F;
	work->light_matrix[1].m[x][y] = 0.0F;
      }

  GM_ConfigObjectLight(&(work->face), work->light_matrix);

  return 0;
}


void dbgFaceMatrix(void)
{
  Work * work;
  int i;
  FVECTOR vec;

  for(i = 0; i < 2; i++)
    {
      work = now_work[i];
      ASSERT(NULL == work);
      DBG("[%d]", i);
      get_joints_vector(&vec, work->face.evmobj, HUMAN21_ATAMA);
      disp_vec(&vec);
    }
}

static int setup_evm(Work *work, int name, int model_id, int speaker_id)
{
  FVECTOR quat;
  int i;

  work->body_move = 0;
  printf("dummy_bone = %d\n", work->dummy_bone);
  CDC_InitObject(&(work->face), work->dummy_bone, DG_FLAG_INVISIBLE);
  CDC_ConfigObjectEvm(&(work->face), model_id, 0, work->side + BASE);

  /* evmobj に対し、表示チャネルの設定を行う */
  /* 左側はチャネル2, 右側はチャネル3 で扱う */
  work->face.evmobj->chanl = work->side + BASE;
  work->face.evmobj->flag  = (!work->side) ? MODEL_FLAG_0 : MODEL_FLAG_1;

  face_lighting(work);  /* 光源初期化 */

  work->name = name;
  work->speaker_id = speaker_id;      /* 顔モーション ID   */
  work->body_id    = speaker_id + 1;  /* 身体モーション ID */

  DBG("speaker ID = %d\n", work->speaker_id);


  /* 基本注視点を設定 */
  get_joints_vector(&work->eye_target, work->face.evmobj, HUMAN21_ATAMA);
  return 0;
}

static int GetResources(Work *work, int name, int dummy_bone, int model_id, int speaker_id)
{
  int i, ix, iy;
  float x, y, w, h;

  work->hair_work = NULL;
  work->dummy_bone = dummy_bone;

  /* カメラ位置移動マトリクスを初期化 */
  for(iy = 0; iy < 4; iy++)
    for(ix = 0; ix < 4; ix++)
      work->camera_work.m[ix][iy] = (float)(iy == ix);

  /* カメラ操作のディレイタイム */
  work->spin_delay = 75.0F / (float)TIME_BASE;
  work->zoom_delay = 300.0F / (float)TIME_BASE;

  /* カメラ位置初期化 */
  work->far = 1000.0F;
  work->zoom = 11.0F;
  work->heading = 0.0F;
  work->pitch   = 0.0F;
  work->pan     = 0.0F;
  work->gain    = 0.0F;

  /* リミッタ初期化 */
  work->heading_right = F_PI / 2.0F;  /*  90° */
  work->heading_left  = F_PI / 2.0F;  /* -90° */
  work->pitch_upper   = F_PI / 4.0F;  /*  45° */
  work->pitch_lower   = F_PI / 4.0F;  /* -45° */
  work->zoom_limit    = 40.0F;  /* ズーム値最大 */

  /* 初期カメラ位置設定 */
  work->now_heading = work->heading;
  work->now_pitch   = work->pitch;
  work->now_zoom    = work->zoom;


  /* ニュートラル状態用の関節リストを用意 */
  work->action_step = STEP_CLOSE;
  work->pre_motion = NULL;
  for(i = 0; i < 64; i++)
    {
      work->nutral[i].vx = 0.0F;
      work->nutral[i].vy = 0.0F;
      work->nutral[i].vz = 0.0F;
      work->nutral[i].vw = 1.0F;
    }

  w = env_list[work->side].w * 512.0F / DRAW_WIDTH;
  h = env_list[work->side].h * 384.0F / DRAW_HEIGHT;
  x = (env_list[work->side].x - env_list[work->side].w / 2) * 512.0F / DRAW_WIDTH;
  y = (env_list[work->side].y - env_list[work->side].h / 2) * 384.0F / DRAW_HEIGHT;

  /* 下地用プレートを用意 */
  setup_bg_plate(work);

  /* かぶせ用フィルタを作成 */
  work->over_filter = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
  ASSERT(work->over_filter != NULL);
  {
    SPR_POS pos;
    SPR_RECT rect;
    float fw, fh;

    pos.x = x;
    pos.y = y;
    work->over_filter->head.pri = 0;
    SPR_SetPosSprite(work->over_filter, &pos);
    SPR_SetSizeSprite(work->over_filter, w, h);
    work->over_filter->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;
    work->over_filter->head.alpha = SCE_GS_SET_ALPHA(0, 2, 2, 1, 100);
    work->over_filter->sprite.col.r = 40;
    work->over_filter->sprite.col.g = 110;
    work->over_filter->sprite.col.b = 90;
    
    work->over_filter->sprite.col.a = 128;
    SPR_SetPriority(work->over_filter, 0);
    SPR_HIDE(work->over_filter);

    fw = (float)DRAW_WIDTH / 512.0F;
    fh = (float)DRAW_HEIGHT / 384.0F;

    for(i = 0; i < 2; i++)
      {
	work->mask_frame[i] =
	  SPR_Create_2D_Object(SP_BOX, 4, work->over_filter);

	SPR_SetPriority(work->mask_frame[i], 0);
	ASSERT(work->mask_frame[i] != NULL);
	rect.begin.x = 0.0F + (float)i * fw;
	rect.begin.y = 0.0F + (float)i * fh;
	rect.end.x = w - (float)(1 + i) * fw;
	rect.end.y = h - (float)(1 + i) * fh;
	SPR_SetPosBox(work->mask_frame[i], &rect);
	SPR_SetColorBox(work->mask_frame[i], 40, 110, 90, 128);
	work->mask_frame[i]->head.flags |= SPR_FLAG_PRIV;
	SPR_SHOW(work->mask_frame[i]);
      }
    
  }

  /* 顔画面にかけるエフェクトを担当する Actor の登録 */
#ifdef EFFECT_CRT
  DBG("FACE CRT Effect.\n");
  GV_SetActorChild(work, NewCodecEffectCRT(work->side,
					   0, 0,
					   env_list[work->side].w,
					   env_list[work->side].h));
#else
  work->effect_works = NULL;
#endif

#ifdef EFFECT_HSYNC 
  work->hsync_work = NewEffectCodecHSyncErrorP(work->side);
  GV_SetActorChild(work, work->hsync_work);
#else
  work->hsync_work = NULL;
#endif

#ifdef EFFECT_HANDYCAM
  GV_SetActorChild(work, work->handy_cam = NewHandyCam());
#else
  work->handy_cam = NULL;
#endif

#ifdef EFFECT_FOCUS
  work->focus_name = GV_StrCode("無線顔") + work->side;
  work->nfocus_work = NewCodecNearFocusEffect(work->focus_name,
					      work->side + BASE, 8, 0, 950);
  GV_SetActorChild(work, work->nfocus_work);

  work->ffocus_work = NewCodecFarFocusEffect(work->focus_name + 5,
					     work->side + BASE, 8, 1100, 2200);
  GV_SetActorChild(work, work->ffocus_work);


  work->focus_cnt = -1; 
  zoom_init(work);
  init_focus(work);
#endif /* EFFECT_FOCUS */

#ifdef EFFECT_FACE_EDGE
  GV_SetActorChild(work, NewEffectCodecFaceEdgeP(work->side));
#endif

  setup_evm(work, name, model_id, speaker_id);
  work->motion_fix = 0;
  work->mtn_camera_cnt = 0;
  return 0;
}

/*****************************************************************************/

/* 指定されたサイドの顔モデルを変更する */
void codecFaceChange(int side, int model_id, int speaker_id)
{
  Work *work;

  /* 指定されたサイドにまだ Work が作られていない場合は何もしない */
  if(NULL == (work = now_work[side])) return;

#ifdef FACE_DEFAULT_MOTION
  /*
   * デフォルトモーションが指定されている場合は、一旦それらを開放する
   */
  destroy_default_motion(work);
#endif

  /*
   * 顔モデルを設定しなおす
   */
  if(work->face.evmobj != NULL)
    {
      /* 髪があれば破棄する。 */
      if(work->hair_work) GV_DestroyActor(work->hair_work);

      /* 顔の EVM モデルが定義されていれば、破棄する */
      CDC_FreeObject(&(work->face));

#ifdef FACE_DEFAULT_MOTION
      destroy_default_motion(work);
#endif /* FACE_DEFAULT_MOTION */
    }
  /* EVM オブジェクトを設定しなおす */
  setup_evm(work, 0, model_id, speaker_id);
}

/*****************************************************************************/

void * NewCodecFace(int name, int dummy_bone, int side, int model_id, int speaker_id)
{
  Work * work;
  
  // OPERATOR();

  if(now_work[side] != NULL)
    {
      codecFaceChange(side, model_id, speaker_id);
      return now_work[side];
    }

  work = (Work *)codecActorPrio(ACT_MODE, sizeof(Work), 0xf0);
  if(work == NULL) return NULL;

  /* 既に同じサイドに割り当てられている Work があれば、それを開放する */
  if(now_work[side] != NULL)
    {
      DBG("Destroy old actor.[%d]\n", side);
      GV_DestroyActor(now_work[side]);
    }

  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);
  /* 表示サイドの設定 */
  work->side = side;

  if(GetResources(work, name, dummy_bone, model_id, speaker_id) < 0)
    {
      GV_DestroyActor(work);
      return NULL;
    }
  work->disp = 0;
  work->nxt_disp = 0;
  work->disp_fade = 0;

  /* 現在の指定サイドの Work として登録 */
  now_work[side] = work;
  return work;
}
