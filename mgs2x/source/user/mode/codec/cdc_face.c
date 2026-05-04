//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
/*
	cdc_face.c
	      無線 顔表示関連
	2000/07/24	Y.Kira
	$Id: cdc_face.c,v 1.7 2002/12/29 12:02:56 takaki Exp $
*/
#endif

#if 0 // BP_Render #ifdef PSX2
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
#endif
#ifdef KP_XBOX
#include <xtl.h>
//BP #include <xbdm.h>
#endif

#include "gameheader.h"
#include "debugmenu.h"
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

#include "c_layout.h"
#include "sprite_2d.h"
#include "3d_util.h"
#include "codecmtn.h"
#include "cdcobj.h"
#include "handycam.h"
#include "c_nfocus.h"
#include "c_ffocus.h"

#include "codec_config.h"
#include "cdc_hair.h"
#include "../../kano/hair/hair_called.h"
#include "cdc_noise.h"

#include "bp_matrix.h"
#include "BP_Camera.h"


/*
  #undef EFFECT_CRT
  #undef EFFECT_HSYNC
  #undef EFFECT_HANDYCAM
  #undef EFFECT_FOCUS
  #undef EFFECT_FACE_EDGE
*/

#if 0 // BP_Render #ifdef PSX2

#undef _DEBUG_ 
#ifdef _DEBUG_
#define DBG(...)  printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* _DEBUG_ */

#endif // PSX2
#if 1 // BP_Render #ifdef KP_XBOX
#define DBG
#endif


#define HAIR_MAX   5   /* 付けられる「髪の毛」の最大数 */


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
//void * NewFaceMotion(int name, DG_EVMOBJ *evmobj, int mouth[7], int eyes);
void   GM_SetLipPower(void *work, float power);
void   DG_SetCamera2(DG_CHANL *chanl,
		     FVECTOR *from, FVECTOR *to, float screen);
int    DG_QueueEvmObj(DG_EVMOBJ * evmobj);
void   DG_DequeueEvmObj(DG_EVMOBJ * evmobj);

void   UTL_StartMemToSpr(void *dst, void *src, int size);
void   UTL_EndMemToSpr(void);

static void set_joints(DG_EVMOBJ * evmobj,
		       int start, int num, FVECTOR * vecarr);

typedef struct {
  FVECTOR root;
  FVECTOR joints[64];
  FVECTOR quat_buffer[4];
  FVECTOR vec;
  FMATRIX mat;
  FMATRIX root_mat;
  FMATRIX mats[64];
} ScrPadWork;


/* モーションデータ管理構造体 */
typedef struct {
  int	    name ;
  int	    count ;
  int	    length ;
  int	    joints ;
  FVECTOR * move ;
  FVECTOR * rots ;
  FVECTOR * trans ;
} MTN_DATA ;



/*
 * 顔オブジェクトに追加する付属品の OBJECT 構造体
 */
typedef struct opt_obj {
  struct opt_obj * prev;   /* 直前の付属品 */
  struct opt_obj * next;   /* 直後の付属品 */

  int     name;            /* 識別コード           */
  int     skel;            /* 基準となる関節の番号 */
  OBJECT  obj;

} OptionObject;


/*
 * 顔表示の前に描画する背景の板(Z 値を 1 以上にするために描画
 */
typedef ALIGN16_DECL(struct) {
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
} BasePlate ;

typedef struct {
  GV_ACT_EX    actor;

  FMATRIX   face_mat;
  FMATRIX   camera_work;  /* 顔カメラ位置マトリクス */
  FVECTOR   camera_pos;   /* カメラ位置目標         */
  FVECTOR   now_camera_pos;   /* カメラ位置現在     */
  FVECTOR   eye_target;   /* 注視点                 */
  FVECTOR   head_pos;     /* 頭部位置               */
  FMATRIX   light_matrix[2];
  FVECTOR   nutral[64];

  int name;

  OBJECT       face;       /* 顔オブジェクト                    */


  OptionObject * begin;     /* オプション付属品リンク始端 */
  OptionObject * end;       /* オプション付属品リンク終端 */

  void       * face_works;  /* 顔アニメーションモジュールの Work 領域
			       へのポインタ */
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
  float     zfar, zoom;     /* カメラ距離とズーム       */
  float     heading, pan;  /* 横方向回転と、平行移動量 */
  float     pitch,   gain; /* 縦方向回転と、平行移動量 */

  /* カメラ操作のディレイタイム */
  float     spin_delay, zoom_delay;

  /* カメラのユーザ操作による補正値 */
  float     adjust_heading; /* heading 操作値 */
  float     adjust_pitch;   /* pitch 操作値   */

  /* カメラの正方向に対する補正値 */
  float     camera_heading;
  float     camera_pitch;   

  /* 現在のカメラ補正値 */
  float     now_camera_heading;
  float     now_camera_pitch;


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


  /* 自動目パチ用パラメータ */
  void    * close_eyes_motion; /* 目パチモーションポインタ */
  MTN_DATA  close_eyes_data;   /* モーションデータ         */
  int       close_eyes_cnt;    /* 目パチ速度カウンタ       */
  int       close_eyes_wait;   /* 目パチ間隔カウンタ       */
  float     close_eyes_rate;   /* 目パチ補間レート         */
  int       close_eyes_step;   /* 目パチシーケンス         */
  void    * hair_work[ HAIR_MAX ];
  int       hair_num;   /* 現在付加している髪の毛の最大数 */

  int       cnt_face_mtn;

  int       force_move;    /* 強制カメラ移動 */

  int       mtn_stream;   /* 非0 なら、ストリーミングモーションが
			     割り当てられている */

  FVECTOR   last_trans[32]; /* 最後の顔モーション値(trans) */
  FVECTOR   last_rots[32];  /* 最後の顔モーション値(rots)  */

  struct {
    FVECTOR   rots;  /* 最後の全身モーション値(rots) */
    FVECTOR   trans; /* 最後の全身モーション値(trans) */
  } body_mtn[21];

  int       close_eyes_enable:1; /* 目パチ許可フラグ */
  int       defmtn_disabe:1;  /* デフォルトモーションの再生を禁止するフラグ */
  int       body_move:1;    /* 体にモーションを割り当てたことがあるか否か   */
  int       pre_face:1;     /* 顔にモーションを割り当てたことがあるか否か   */
  int       canceled:1;     /* キャンセルしたことがあるか否か               */
  int       camera_ctrl:1;  /* カメラ操作の許可/禁止 */
  int       bug_eyes:1;     /* 白目フラグ */
  int       trace_camera:1; /* 0: 追従カメラ OFF / 1: 追従カメラ ON */

#ifdef DEBUG_MODE
  /* デバッグモードのライト位置変更用ワーク */
  float     light_angle;
  float     ring_angle;
#endif /* DEBUG_MODE */

} Work;



enum {
  STEP_CLOSE,     /* 閉じ状態                             */
  STEP_OPENING,   /* CRT の走査線が広がる最中             */
  STEP_FOCUS,     /* ズーム/フォーカシング                */
  STEP_OPENED,    /* 表示完了                             */
  STEP_CLOSING    /* 走査線が縮む                         */
};


static void set_focus_area(Work * work, int zone);
DEMO_MOTION * GM_StreamGetMotion( int id );

#define  ACT_X_TIME   15.0F  /* x 方向に走査線が広がりきるまでの時間 */
#define  ACT_Y_DELAY  13.0F  /* y 方向が広がり始めるまでの遅延時間   */
#define  ACT_Y_TIME   80.0F /* y 方向が広がり始めてから広がりきるまでの時間 */

#define ZOOM_LIMIT_UNDER 9.0F  /* 最小ズーム値 */

static int face_flag[2];      /* チャネル 0,1 の表示状態保存用 */
static int face_chanl_gpid[2];
static int bgclear_flag[2];   /* チャネル 0,1 のbg_clear_flag 保存用 */

/* 両サイドで現在有効になっている Work */
static Work    * now_work[2] = {NULL, NULL};


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



#ifdef DEBUG_MODE

static int  _DEBUG_codec_face = 0;  /* デフォルトは OFF */
static int  _DEBUG_face_guide = 0;  /* デフォルトは OFF */
static int  _DEBUG_face_camera = 0; /* デフォルトは OFF */

#ifdef PSX2
static GM_DEBUG_MENU debug_menu = {
  class:      "CODEC",
  menu:       "FACE",
  max:        4,
  items: (char *[]){"CAMERA", "LIGHT", "OFF", "WATCH"},
  values: (int[]){ 1, 2, 0, 3},
  target: &_DEBUG_codec_face,
  mask: 0x00000003
};

static GM_DEBUG_MENU debug_menu2 = {
  class:      "CODEC",
  menu:       "FACE GUIDE RECT",
  max:        2,
  items: (char *[]){"ON", "OFF"},
  values: (int[]){ 1, 0},
  target: &_DEBUG_face_guide,
  mask: 0x00000001
};

static GM_DEBUG_MENU debug_menu3 = {
  class:      "CODEC",
  menu:       "FACE CAMERA INFO",
  max:        2,
  items: (char *[]){"ON", "OFF"},
  values: (int[]){ 1, 0},
  target: &_DEBUG_face_camera,
  mask: 0x00000001
};

void CODEC_AddFaceDebugMenu(void)
{
  GM_AddDebugMenu(&debug_menu);
  GM_AddDebugMenu(&debug_menu2);
  GM_AddDebugMenu(&debug_menu3);
}
#endif  // PSX2
#ifdef KP_XBOX
void CODEC_AddFaceDebugMenu(void)
{
}
#endif

#endif /* DEBUG_MODE */



static void soft_focus_switch(Work * work, int sw);

/*
 * 登録されている全ての付属品オブジェクトを開放する
 */
static void free_all_object(Work * work)
{
  OptionObject * opt;
  OptionObject * next;

  opt = work->begin;
  while(NULL != opt)
    {
      next = opt->next;
      CDC_FreeObject(&(opt->obj));
      codecDelayedFree(opt);
      opt = next;
    }
  work->begin = NULL;
  work->end   = NULL;
}

void codecBugEyesSwitch(int side, int sw)
{
  Work * work = now_work[side];

  if(NULL == work) return;
  work->bug_eyes = sw;
}

void codecEyeAnimationSwitch(int side, int name)
{
  Work * work = now_work[side];
  int sw = 1;

  if(NULL == work) return;    /* 顔が無ければなにもしない */

  /* 指定された話者が自分と同じであれば、デフォルト目パチを禁止する */
  if(name == work->speaker_id) sw = 0;  
  work->close_eyes_enable = sw;
}

/*
 * レバーによるカメラ操作の禁止/許可を設定する
 */
void codecCameraControlSwitch(int side, int sw)
{
  Work * work = now_work[side];

  if(NULL == work) return;

  work->camera_ctrl = sw;
}

/*
 * 目パチモーションを指定
 */
void codecSetEyeCloseMotion(int side, int name)
{
  Work * work = now_work[side];
  RMT_FILE_HEADER * mtn;
  MTN_DATA * data = &(work->close_eyes_data);
  int size;

  if(NULL == work) return;    /* 顔が無ければなにもしない */

  /* 非圧縮の生モーション(最初の1フレームのみ,顔だけ、目だけが閉じている) */

  if(NULL == (mtn = GV_GetCache(GV_CacheID(name, 'r'))))
  {
    mtn = CDC_GetFileEntry(name, 'r');
  }
  ASSERT(mtn != NULL);

  RMT_VerifyEndianSwap( mtn );

  data->length = mtn->motion_length;  /* 長さ(関係ない)     */
  data->joints = mtn->motion_joints;  /* 関節数(32以外ダメ) */
  size = 0;
  data->move = (FVECTOR *)((char *)&mtn[1] + size);
  size = mtn->move_size;
  data->rots = (FVECTOR *)((char *)&mtn[1] + size);
  size = mtn->rots_size;
  data->trans = (FVECTOR *)((char *)&mtn[1] + size);
  size = mtn->trans_size;
  data->count = 1;

  work->close_eyes_motion = mtn;
  work->close_eyes_cnt  = 0;
  work->close_eyes_wait = 0;
  work->close_eyes_rate = 0.0F;
  work->close_eyes_step = EYES_STAT_OPEN;
}

/* ベクトルの線形補間 */
static void InterpVector( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t )
{
#ifdef BP_PSX2_ASM
  asm ("
		mfc1		$8,%3
		lqc2		vf1,0(%1)
		lqc2		vf2,0(%2)
		qmtc2		$8,vf3
		vmulax.xyz	ACC,vf2,vf3
		vmsubax.xyz	ACC,vf1,vf3
		vmaddw.xyz	vf1,vf1,vf0
		sqc2		vf1,0(%0)
	"::"r"(res),"r"(from),"r"(to),"f"(t): "$8" );
#else
  res->vx = ( to->vx - from->vx ) * t + from->vx ;
  res->vy = ( to->vy - from->vy ) * t + from->vy ;
  res->vz = ( to->vz - from->vz ) * t + from->vz ;
#endif
}

/*
 * レートで補間して、目の関節のみを設定する
 */
static void EyeMotionSet(Work * work, float rate)
{
  int i;
  DG_EVMOBJ  * evmobj = work->face.evmobj;
  ScrPadWork * scrpad = (ScrPadWork *)SCRPAD_ADDR;
  FMATRIX    * skel_mats;
  FVECTOR      vec;
  FVECTOR      trans, t_vec;
  FVECTOR      rot,   r_vec;
  FMATRIX    * mats = scrpad->mats;
  FMATRIX    * parent;
  EVM_SKEL   * skel;
  MTN_DATA   * mtn = &(work->close_eyes_data);

  skel_mats = evmobj->matrix[ evmobj->use_buffer ];
  skel = &evmobj->def->skeleton[ 45 ];  /* まぶた周辺関節のみ */
  vec.vw = 1.0F;

  for(i = 24; i < 32; i++)
    { 
      rot   = mtn->rots[ (mtn->length + 1) * i + mtn->count ];
      trans = mtn->trans[ (mtn->length + 1) * i + mtn->count ];
      rot.vw = 1.0F;
      trans.vw = 1.0F;

      /* 関節が値を持っているもの以外設定しない */
      if((trans.vx != 0.0F) || (trans.vy != 0.0F) || (trans.vz != 0.0F) ||
	 (rot.vx != 0.0F) || (rot.vy != 0.0F) || (rot.vz != 0.0F))
	{
	  /*
	   * 補間を行う
	   */
#ifdef BP_PSX2_GCC
	  InterpVector(&t_vec,
		       &(FVECTOR){0.0F, 0.0F, 0.0F, 1.0F}, &trans, rate);
	  MT_QuatSlerp(&r_vec,
		       &(FVECTOR){0.0F, 0.0F, 0.0F, 1.0F}, &rot, rate);
#else //BP
//#endif
//#ifdef KP_XBOX
	  FVECTOR pos = {0.0F, 0.0F, 0.0F, 1.0F};
	  InterpVector(&t_vec,
		       &pos, &trans, rate);
	  MT_QuatSlerp(&r_vec,
		       &pos, &rot, rate);
#endif
	  MT_QuatNormalize(&r_vec, &r_vec);
	  
	  MT_QuatToMat(mats, &r_vec);
	  mats->m[3][0] = skel->rt_tx + t_vec.vx;
	  mats->m[3][1] = skel->rt_ty + t_vec.vy;
	  mats->m[3][2] = skel->rt_tz + t_vec.vz;
	  
	  parent = &skel_mats[ skel->parent ];
	  _sceVu0MulMatrix( mats, parent, mats );
	  vec.vx = -skel->rt_tx;
	  vec.vy = -skel->rt_ty;
	  vec.vz = -skel->rt_tz;
	  _sceVu0ApplyMatrix( &mats->m[3][0], mats, &vec );
	  skel_mats[i + 21] = *mats;
	}
      skel++;
      mats++;
    }
}

/*
 * 目パチのシーケンスを管理する
 */
static void EyeAnimation(Work * work)
{
  if(NULL == work->close_eyes_motion) return;
  if(!work->close_eyes_enable) return;  /* 目パチ禁止ならなにもしない */
  if(work->bug_eyes) return;  /* 白目を剥いているならなにもしない */

  switch(work->close_eyes_step)
    {
    case EYES_STAT_OPEN:
      work->close_eyes_wait -= TIME_BASE;
      if(work->close_eyes_wait <= 0)
	{
	  work->close_eyes_step = EYES_STAT_CLOSING;
	  work->close_eyes_cnt = 0;
	}
      work->close_eyes_rate = 0.0F;
      break;
    case EYES_STAT_CLOSING:
      work->close_eyes_cnt += TIME_BASE;
      work->close_eyes_rate =
	(float)work->close_eyes_cnt / (float)EYES_CLOSE_SPEED;
      if(work->close_eyes_rate >= 1.0F) work->close_eyes_rate = 1.0F;
      if(work->close_eyes_cnt >= EYES_CLOSE_SPEED)
	work->close_eyes_step = EYES_STAT_CLOSED;
      break;
    case EYES_STAT_CLOSED:
      work->close_eyes_rate = 1.0F;
      work->close_eyes_step = EYES_STAT_OPENING;
      work->close_eyes_cnt = 0;
      break;
    case EYES_STAT_OPENING:
      work->close_eyes_cnt += TIME_BASE;
      work->close_eyes_rate =
	(float)work->close_eyes_cnt / (float)EYES_CLOSE_SPEED;
      if(work->close_eyes_rate >= 1.0F) work->close_eyes_rate = 1.0F;
      work->close_eyes_rate = 1.0F - work->close_eyes_rate;
      if(work->close_eyes_cnt >= EYES_OPEN_SPEED)
	{
	  work->close_eyes_step = EYES_STAT_OPEN;
	  work->close_eyes_wait = EYES_BLANK_MIN +
	    ((BP_PS2_rand() >> 16) % EYES_BLANK_RND);
	}
      break;
    }
  EyeMotionSet(work, work->close_eyes_rate);
}


/*
 * 指定された胸像モデルに,付属品を付加する
 */
int cdcFaceSetObject(int side, int model, int spk_id, int skel_num)
{
  Work * work = now_work[side];
  OptionObject * optobj;
  int i;


  printf("cdcFaceSetObject: side = %d\n", side);

  if(NULL == work) return -1;

  printf("cdcFaceSetObject(): model = 0x%08x\n", model);

  /* オプションオブジェクトを保持する領域を確保する */
  optobj = codecMalloc(sizeof(OptionObject));
  ASSERT(NULL != optobj);

  optobj->name = spk_id;   /* モーション識別用 ID */
  optobj->skel = skel_num; /* 指定された関節に関連付ける */

  /* オブジェクトの用意 */

  /*
    CDC_InitObject(&(optobj->obj), model,
    DG_FLAG_FINISHCALC | DG_FLAG_SHADE, side + 2);
  */
  CDC_InitObject(&(optobj->obj), model,
		 DG_FLAG_SEMITRANS | DG_FLAG_LATTERDRAW |
		 DG_FLAG_FINISHCALC | DG_FLAG_SHADE | DG_FLAG_NOFOG,
		 side + 2);
  
  /*
    optobj->obj.flag &= ~((!side) ? DG_FLAG_INVISIBLE2 : DG_FLAG_INVISIBLE3);
    for(i = 0; i < optobj->obj.objs->def->n_x_models; i++)
    optobj->obj.objs->objs[i].flag &=
    ~((!side) ? DG_FLAG_INVISIBLE2 : DG_FLAG_INVISIBLE3);
  */
  /* 余計なデータは NULL にしておく */
  optobj->obj.evmobj = NULL;
  optobj->obj.m_ctrl = NULL;
  printf("(%f %f %f)-(%f %f %f)\n",
	 optobj->obj.objs->objs[0].bound_min.vx,
	 optobj->obj.objs->objs[0].bound_min.vy,
	 optobj->obj.objs->objs[0].bound_min.vz,
	 optobj->obj.objs->objs[0].bound_max.vx,
	 optobj->obj.objs->objs[0].bound_max.vy,
	 optobj->obj.objs->objs[0].bound_max.vz);

  optobj->obj.objs->flag &=
    ~((!side) ? DG_FLAG_INVISIBLE2 : DG_FLAG_INVISIBLE3);
  optobj->obj.objs->light = work->light_matrix;

  for(i = 0; i < optobj->obj.objs->def->n_x_models; i++)
    {
      optobj->obj.objs->objs[i].flag &=
	~((!side) ? DG_FLAG_INVISIBLE2 : DG_FLAG_INVISIBLE3);
      optobj->obj.objs->objs[i].light = work->light_matrix;
    }

  

  /* 完成したオプションオブジェクト構造体を,リンクに追加する */
  optobj->next = NULL;
  optobj->prev = work->end;
  if(NULL != work->end) work->end->next = optobj;
  work->end = optobj;
  if(NULL == work->begin) work->begin = optobj;
  
  return 0;
}

#if 1 //BP def DEBUG
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


#ifdef DEBUG_MODE

/*
 * ライトの角度を変化させる
 */
static void debug_change_light(Work * work)
{
  FMATRIX mat;
  FVECTOR vec;
  int x, y;

  /*
   * 原点ベクトルを作成
   */
  for(y = 0; y < 4; y++)
    for(x = 0; x < 4; x++)
      mat.m[x][y] = (float)(x == y);

  /* y 軸を中心として回転 */
  _sceVu0RotMatrixY(&mat, &mat, work->light_angle);

  /* x 軸を中心として回転 */
  _sceVu0RotMatrixX(&mat, &mat, work->ring_angle);

  /* 光源回転用のベクトルを,正面からの光源ベクトルにかける */
  vec.vx = vec.vy = 0.0F;
  vec.vz = -1.0F;
  vec.vw = 1.0F;

  _sceVu0ApplyMatrix(&vec, &mat, &vec);

  codecSetFaceLightVector(work->side, vec.vx, vec.vy, vec.vz);

  if((GV_PadDataDirect[0].press & (PAD_R1 | PAD_R2)) == (PAD_R1 | PAD_R2))
    printf("lightvec %d  %d %d %d\n", work->side,
	   (int)(vec.vx * 1000), (int)(vec.vy * 1000), (int)(vec.vz * 1000));
}

#endif /* DEBUG_MODE */




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

#ifdef DEBUG_MODE
  switch(_DEBUG_codec_face)
    {
    case 1:   /* CAMERA */
      {
	int status = GV_PadDataDirect[0].status & (PAD_L1|PAD_L2);
	int press =  GV_PadDataDirect[0].press & (PAD_R1|PAD_R2);
	/*
	 * ボタンを押しながらの場合は,
	 * カメラアングルではなくフレーミングの移動
	 * (上下左右シフト)
	 */
	if(zsw) work->body_move = 0;
	
	switch(status)
	  {
	  case (PAD_L1|PAD_L2):    /* 同時押しで距離 */
	    work->zfar -= fy / 10.0F;
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
	
	if((PAD_R2 | PAD_R1) == press)
	  printf("facecamera %d %d %d %d %d %d %d\n",
		 work->side,
		 (int)(work->zfar * 1000.0F),
		 (int)(work->zoom * 1000.0F),
		 (int)(work->heading * 2048.0F / F_PI),
		 (int)(work->pan * 1000.0F),
		 (int)(work->pitch * 2048.0F / F_PI),
		 (int)(work->gain * 1000.0F));
      }
      break;
    case 2:    /* LIGHT */
      {
	/* アナログレバー操作でライトマトリクスを設定する */
	
	/* 左右方向への回転 */
	work->light_angle += fx / (F_PI * ANALOG_MAX * 8);
	if(work->light_angle > F_PI) work->light_angle = F_PI;
	if(work->light_angle < -F_PI) work->light_angle = -F_PI;

	/* 上下方向への回転 */
	work->ring_angle += fy / (F_PI * ANALOG_MAX * 8);
	if(work->ring_angle > (F_PI / 2.0F))  work->ring_angle = F_PI / 2.0F;
	if(work->ring_angle < (-F_PI / 2.0F)) work->ring_angle = -F_PI / 2.0F;

	/* 現在の光源角度に応じて光源ベクトルを回転し、再設定する */
	debug_change_light(work);
	work->target_heading = work->heading;
	work->target_pitch   = work->pitch;
      }
      break;
    case 0:    /* OFF */
#endif

      /* 顔の回転(左右は最大±90°、上下は±45°) */
      h_max = (fx >= 0) ? work->heading_right : work->heading_left;
      p_max = (fy >= 0) ? work->pitch_lower : work->pitch_upper;
      
      if(work->camera_ctrl)
	{
	  work->adjust_heading = h_max * fx / (float)ANALOG_MAX;
	  work->adjust_pitch   = p_max * fy / (float)ANALOG_MAX;
	}
      else
	{
	  work->adjust_heading = 0.0F;
	  work->adjust_pitch   = 0.0F;
	}

      work->target_heading = work->heading;
      work->target_pitch   = work->pitch;

      /* 目標カメラ角を設定 */
      work->camera_heading = -work->adjust_heading;
      work->camera_pitch   = -work->adjust_pitch;

      /* 現在のカメラ角と補正後の角度間の補間 */
      work->now_camera_heading = work->now_camera_heading + 
	(work->camera_heading - work->now_camera_heading) / work->spin_delay;
      work->now_camera_pitch = work->now_camera_pitch + 
	(work->camera_pitch - work->now_camera_pitch) / work->spin_delay;

#ifdef DEBUG_MODE
      break;
    }
#endif /* DEBUG_MODE */

  /*
   * ズーム
   */
  target_zoom = (!zsw) ? work->zoom : work->zoom_limit;

  /* 現在の回転角から、目標回転角までの角度を補間する */
  work->now_heading = work->now_heading +
    (work->target_heading - work->now_heading) / work->spin_delay;
  work->now_pitch = work->now_pitch +
    (work->target_pitch - work->now_pitch) / work->spin_delay;

#ifdef DEBUG_MODE
  if(_DEBUG_codec_face == 1)
    {
      /* カメラ角度がレバーを離しても固定されるようにする */
      work->heading = work->now_heading;
      work->pitch   = work->now_pitch;
    }
#endif /* DEBUG_MODE */

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
	  work->focus_zone = (int)(work->zfar + FOCUS_FUNC(rate) * work->zfar);
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
  work->camera_pos.vz = work->zfar;
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
 * ストリーミングキャンセル時に呼ぶ
 */
int cdcFaceVoiceCancel(int side)
{
  Work * work = now_work[side];

  if(NULL == work) return -1;
  work->canceled = 1;
  work->pre_motion = NULL;
  return 0;
}

/*
 * トレースカメラスイッチ
 */
int cdcTraceCameraSwitch(int side, int sw)
{
  Work * work = now_work[side];
  if(NULL == work) return -1;

  work->trace_camera = sw;

  return 0;
}

/*
 * 無線顔カメラ位置設定
 */
int cdcFaceCamera(int side,
		  float zfar, float zoom,
		  float heading, float pan,
		  float pitch,   float gain)
{
  Work * work = now_work[side];

  if(NULL == work) return -1;

  work->heading  = heading;
  work->pan      = pan;
  work->pitch    = pitch;
  work->gain     = gain;
  work->zfar      = zfar;
  work->zoom     = zoom;
 
  work->now_zoom    = zoom;
  work->now_heading = heading;
  work->now_pitch   = pitch;

  work->camera_heading = 0.0F;
  work->camera_pitch = 0.0F;

  work->now_camera_heading = 0.0F;
  work->now_camera_pitch   = 0.0F;

  work->target_heading = heading;
  work->target_pitch = pitch;

  work->adjust_heading = 0.0F;
  work->adjust_pitch = 0.0F;

  printf("Set Camera: heading = %f  pitch = %f  zoom = %f\n", heading, pitch, zoom);

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
  int near, zfar;

  near = zone - 20;
  zfar = zone + 20;
#ifdef EFFECT_FOCUS
  nfocusSetFocus(work->nfocus_work, 0, 0, near - 100, near);
  ffocusSetFocus(work->ffocus_work, 0, 0, zfar, zfar + 100);
#endif /* EFFECT_FOCUS */
}

static void init_focus(Work * work)
{
  float rate;

  work->focus_time = (int)((float)work->zoom_time / 0.8F);
  work->focus_cnt = 0;
  // ASSERT(work->focus_time > 0);
  if(work->focus_time)
    {
      rate = (F_PI - 1.0F) * (float)work->focus_cnt / (float)work->focus_time + 1.0F;
      work->focus_zone = (int)(work->zfar + FOCUS_FUNC(rate) * work->zfar);
      DBG("focus_zone = %d\n", work->focus_zone);
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
  work->focus_zone = (int)(work->zfar + FOCUS_FUNC(rate) * work->zfar);
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
  work->zoom_adjust_base = (float)((BP_PS2_rand() >> 16) % 3000) / -1000.0F;
  a = (BP_PS2_rand() >> 16) & 1;
  work->zoom_goal = (a) ? F_PI : (F_PI * 4 / 3);
  work->zoom_time = ZOOM_TIME + ((BP_PS2_rand() >> 16) % 20) - 10;
  work->zoom_wait = 0; /* ZOOM_WAIT + ((BP_PS2_rand() >> 16) % 10) - 5; */
  
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
 * 背景板設定
 */
#define DMA_DISABLE (DG_DMAPACK_INVISIBLE0 | DG_DMAPACK_INVISIBLE1 | \
                       DG_DMAPACK_INVISIBLE2 | DG_DMAPACK_INVISIBLE3)

static int setup_bg_plate(Work * work)
{
#if 0 // BP_Render #ifdef PSX2

   static BasePlate def_plate = {
#if 1 //BP_GCC

      //dmatag
      {
         DMATAG_SET_QWC(DMATAG_ID_RET, Qsize(struct _base_gif)), //qwc
            NULL, //addr
         { SCE_VIF1_SET_NOP(0), SCE_VIF1_SET_DIRECT(Qsize(struct _base_gif), 0) } //vifcode
      },
         //gif
      {
         { SCE_GIF_SET_TAG(Qsize(struct _base_data), 1, 0, 0, 0, 1), GS_REGS_AD }, //giftag
            //data
         {
            { 0, SCE_GS_ZBUF_1 }, //zbuf0
            { SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1), SCE_GS_TEST_1 }, //test0
            { SCE_GS_SET_ALPHA(0, 1, 0, 1, 128), SCE_GS_ALPHA_1 }, //alpha
            { SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0), SCE_GS_PRIM }, //prim
            { SCE_GS_SET_RGBAQ(FACEBG_R, FACEBG_G, FACEBG_B, 128, 0), SCE_GS_RGBAQ }, //rgbq0
            { 0, SCE_GS_XYZ2 }, //xyz0
            { SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ(FACEBG_R, FACEBG_G, FACEBG_B, 128, 0) }, //rgbq1
            { 0, SCE_GS_XYZ2 }, //xyz1
            { 0, SCE_GS_TEST_1 }, //zbuf1
            { 0, SCE_GS_ZBUF_1 }//test1
         }
      }
#else

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
	  // data: SCE_GS_SET_ALPHA(0, 2, 2, 2, 128)
	  data: SCE_GS_SET_ALPHA(0, 1, 0, 1, 128)
	},
	prim:
	{
	  reg: SCE_GS_PRIM,
	  data: SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0)
	},
	rgbq0:
	{
	  reg: SCE_GS_RGBAQ,
	  data: SCE_GS_SET_RGBAQ(FACEBG_R, FACEBG_G, FACEBG_B, 128, 0)
	  // data: SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0)
	},
	xyz0: { reg: SCE_GS_XYZ2 },
	rgbq1:
	{
	  reg: SCE_GS_RGBAQ,
	  data: SCE_GS_SET_RGBAQ(FACEBG_R, FACEBG_G, FACEBG_B, 128, 0)
	  // data: SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0)
	},
	xyz1: { reg: SCE_GS_XYZ2 },
	test1: { reg: SCE_GS_TEST_1 },
	zbuf1: { reg: SCE_GS_ZBUF_1 }
      }
    }
#endif //BP_GCC
  };
  int i, w, h, flags;
  u_long64 zbuf, test;
  DG_DMAPACK * dmapack;


  DBG("Setup Face BG!!\n");
  work->plate = codecMalloc(sizeof(BasePlate) * 2);

  ASSERT(work->plate != NULL);

  w = FACE_WIDTH;
  h = FACE_HEIGHT;

  for(i = 0; i < 2; i++)
    {
      DBG("w = %d, h = %d\n", w, h);

      work->plate[i] = def_plate;
      work->plate[i].gif.data.xyz0.data = SCE_GS_SET_XYZ((2048 - w / 2) * 16,
							 (2048 - h / 2) * 16,
							 1);
      work->plate[i].gif.data.xyz1.data = SCE_GS_SET_XYZ((2048 + w / 2) * 16,
							 (2048 + h / 2) * 16,
							 1);

      zbuf = *((u_long64 *)&DG_Chanls[work->side + 2].draw_env[i].datas.zbuf1);
      test = *((u_long64 *)&DG_Chanls[work->side + 2].draw_env[i].datas.test1);

      DBG("zbuf = %016lx   test = %016lx\n", zbuf, test);
      work->plate[i].gif.data.zbuf0.data = zbuf & ~I64(0x100000000) ;

      /* 描画環境復元設定 */
      work->plate[i].gif.data.zbuf1.data = zbuf;
      work->plate[i].gif.data.test1.data = test;
    }
  
  // dmapack->flag = flags;
  dmapack = DG_MakeDmapack(DG_DMAPACK_NORMAL | DG_DMAPACK_PRIVILEGE,
			   DG_DMAPACK_PHASE_FIRST);
  
  
  dmapack->flag &= ~DMA_DISABLE;
  dmapack->flag |= (DMA_DISABLE &
		    ~((!work->side)
		      ? DG_DMAPACK_INVISIBLE2
		      : DG_DMAPACK_INVISIBLE3));

  dmapack->packet[0] = &(work->plate[0]);
  dmapack->packet[1] = &(work->plate[1]);
  ASSERT(dmapack != NULL);
  work->plate_dmapack = dmapack;
  DG_QueueDmapack(dmapack);
#else //XBOX

  DG_DMAPACK * dmapack;

  work->plate = codecMalloc(256);
  ASSERT(work->plate != NULL);
  dmapack = DG_MakeDmapack(DG_DMAPACK_NORMAL | DG_DMAPACK_PRIVILEGE,
						   DG_DMAPACK_PHASE_FIRST);
  dmapack->flag &= ~DMA_DISABLE;
  dmapack->flag |= (DMA_DISABLE &
		    ~((!work->side)
		      ? DG_DMAPACK_INVISIBLE2
		      : DG_DMAPACK_INVISIBLE3));

  dmapack->autopacket = work->plate;
  work->plate_dmapack = dmapack;
  DG_SetDmapackEnd( dmapack->autopacket );
  DG_QueueDmapack( dmapack );

#endif 
  return 0;
}

/*
 * モーション割当のステータスを得る
 */
int codecGetMotionStatus(int side)
{
  Work * work = now_work[side];

  if(NULL == work) return 0;   /* ストリーミングモーションなどあるわけがない */

  return work->mtn_stream;
}


/*
 * 髪の毛の設定(ライデン、ローズ用)
 */
void codecSetupHair(int side,
		    void * model, int sample_num,
		    void * bound, int hit_param, int ambient, int nowind)
{
  Work * work = now_work[side];
  void * hair_work;

  ASSERT(NULL != work);
  /*
    if(NULL == bound)
    bound = GV_GetCache(GV_CacheID(DUMMY_BONE, 'k'));
  */
#ifdef DEBUG
  printf("model = %p\n", model);
  printf("bound = %p\n", bound);
  printf("sample_num = %d\n", sample_num);
  printf("ambient = %d\n", ambient);
  printf("chanl = %d\n", BASE + side);
#endif /* DEBUG */


  /* 髪の毛設定 */

  hair_work = NewEvmHairModel_Wireless(0,               /* name */
				       model,           /* evm  */
				       bound,           /* bound */
				       sample_num,      /* sample_num */
				       &(work->face),   /* target */
				       ambient,         /* light_flag */
				       BASE + side,     /* chanl */
				       DELAY_DRAW,      /* delay flag */
				       nowind);         /* nowind */

  ASSERT(hair_work != NULL);
  printf("Hair Setup!!\n");
  GV_SetActorChild(work, hair_work);
  work->hair_work[work->hair_num] = hair_work;
  work->hair_num++;

  /* 髪の毛計算フラグをリセット */
  GV_CallChildSignalFunc(work, HAIR_SIGNAL_CALC_FLAG, 0);
  work->cnt_face_mtn = 0;
}

/*
 * 顔表示初期化
 */
void codecSetupScreen(void)
{
  int i;

  /* チャネル 0, 1 の表示状態保存および非表示化 */
  for(i = 0; i < 2; i++)
    {
      face_flag[i] = DG_Chanls[i].flag;   /* 表示状態の保存 */
      bgclear_flag[i] = DG_Chanls[i].bg_clear_flag;
    }

  /* チャネル 0 は非表示。 */
  DG_Chanls[0].flag = 0;
  // DG_Chanls[0].bg_clear_flag = 1;  /* 毎フレーム背景を消去 */


  /* チャネル 1 も非表示 */
  DG_Chanls[1].flag = 0;

  /* チャネル 2, 3 を初期化するが、まだ表示状態にはしない */
  for(i = 0; i < 2; i++)
    {
#if 0
      /* 表示領域の設定 */
      DG_SetDrawEnv(&DG_Chanls[i + BASE],
		    (env_list[i].x - DRAW_WIDTH / 2),
		    (env_list[i].y - DRAW_HEIGHT / 2),
		    env_list[i].w, env_list[i].h);
      
      /* カメラの設定 */
      BP_Camera_SetCodecCamera();  //BP_CAMERA - set active camera for wide screen tweak system
      /* DG_SetCamera2(&DG_Chanls[i + BASE], &eye[i], &center[i], def_zoom[i]); */
#endif
      /* 表示状態に移行 */
      DG_Chanls[i + BASE].flag = 0;
      DG_Chanls[i + BASE].bg_clear_flag = 1;  /* 毎フレーム背景を消去 */
      face_chanl_gpid[i] = DG_Chanls[i+BASE].group_id;
      DG_Chanls[i + BASE].group_id = -1;    // すべて表示
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
    {
      printf("codecRecoveryScreen(): face_flag[%d] = %d\n", i, face_flag[i]);
      DG_Chanls[i].flag = face_flag[i];
      DG_Chanls[i].bg_clear_flag = bgclear_flag[i];
      DG_Chanls[i+BASE].group_id = face_chanl_gpid[i];
    }
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

  objs->world = evmobj->world;
}


/*
 * デフォルトモーション再生
 */
static void play_default_motion(Work * work)
{
#ifdef BP_PSX2_GCC
  SVECTOR rot;
  FVECTOR mov;

  GM_ActMotion(&(work->face));
  // 原点に設定
  mov = (FVECTOR){0.0F, 0.0F, 0.0F, 0.0F};
  rot = (SVECTOR){0, 0, 0, 0};
#else //BP
//#endif
//#ifdef KP_XBOX
  SVECTOR rot = {0, 0, 0, 0};
  FVECTOR mov = {0.0F, 0.0F, 0.0F, 0.0F};

  GM_ActMotion(&(work->face));
  // 原点に設定
#endif
  DG_SetPos2(&mov, &rot);
  GM_ActObject2(&(work->face));

  /* EVM のマトリクスを OBJ に書き戻す */
  syncro_obj(work);
}


/*
 * 全身モーションを直前のフレームで固定する
 */
static void hold_body_motion(Work * work)
{
  int i;

  DG_Arm_SwitchEvmBuffer(work->face.evmobj);
  set_joints(work->face.evmobj, 0, 21, (FVECTOR *)work->body_mtn);
  syncro_obj(work);
}


/*
 * デフォルトモーション開放
 */
static void destroy_default_motion(Work * work)
{
}
#endif /* FACE_DEFAULT_MOTION */


/*
 * 最後に設定した顔モーション関節値を保存する
 */
static void save_face(Work * work, int start, int n_joints, FVECTOR * mtn)
{
  int i;
  int base;

  base = 21 - start;
  for(i = 0; i < 32; i ++)
    {
      /* 関節の rotation と translation を保存 */
      work->last_rots[i] = *(mtn +(base + i) * 2);
      work->last_trans[i] = *(mtn + (base + i) * 2 + 1);
    }
}

/*
 * 最後に設定した全身モーション関節値を保存する
 */
static void save_body_motion(Work * work,
			     int start, int n_joints, FVECTOR * mtn)
{
  int i;

  for(i = 0; i < 21; i++)
    {
      work->body_mtn[i].rots = *(mtn + i * 2);
      work->body_mtn[i].trans = *( mtn + i * 2 + 1);
    }
}

/*
 * EVM モデルの直前の関節を継承する
 */
static void cont_joints(Work * work, int start, int num)
{
  int i;
  DG_EVMOBJ * evmobj = work->face.evmobj;
  FMATRIX * skel_mats;
  FVECTOR   vec;
  FMATRIX * mats;
  EVM_SKEL * skel;
  FVECTOR * rot;
  FVECTOR * trans;

  skel_mats = evmobj->matrix[ evmobj->use_buffer ];
  skel = evmobj->def->skeleton + start;
  vec.vw = 1.0F;
  for(i = 0; i < num; i++, skel++)
    {
      // if(!(i + start)) continue;
      mats = &skel_mats[i + start];
      /* 直前のクォータニオンとトランスレーションの取得 */
      rot   = &work->last_rots[i];
      trans = &work->last_trans[i];
      
      MT_QuatToMat(mats, rot);
      mats->m[3][0] = skel->rt_tx + trans->vx;
      mats->m[3][1] = skel->rt_ty + trans->vy;
      mats->m[3][2] = skel->rt_tz + trans->vz;

      if(skel->parent != -1)
	_sceVu0MulMatrix(mats, &skel_mats[ skel->parent ], mats);
      else
	_sceVu0MulMatrix(mats, &DG_UnitMatrix, mats);
      
      vec.vx = -skel->rt_tx;
      vec.vy = -skel->rt_ty;
      vec.vz = -skel->rt_tz;
      _sceVu0ApplyMatrix((FVECTOR *)mats->m[3], mats, &vec);
    }
}

/*
 * EVMモデルに,関節を割り当てる
 */
static void set_joints(DG_EVMOBJ * evmobj,
		       int start, int num, FVECTOR * vecarr)
{
  int i;
  FMATRIX * skel_mats;
  FVECTOR   vec;
  FMATRIX * mats;
  EVM_SKEL * skel;
  FVECTOR * rot;
  FVECTOR * trans;

  skel_mats = evmobj->matrix[ evmobj->use_buffer ];
  skel = evmobj->def->skeleton + start;
  vec.vw = 1.0F;

  for(i = 0; i < num; i++, skel++)
    {
      // if(!(i + start)) continue;

      mats = &skel_mats[i + start];
      /* クォータニオンとトランスレーションの取得 */
      rot   = &vecarr[i * 2];
      trans = &vecarr[i * 2 + 1];
      
      MT_QuatToMat(mats, rot);
      mats->m[3][0] = skel->rt_tx + trans->vx;
      mats->m[3][1] = skel->rt_ty + trans->vy;
      mats->m[3][2] = skel->rt_tz + trans->vz;

      if(skel->parent != -1)
	_sceVu0MulMatrix(mats, &skel_mats[ skel->parent ], mats);
      else
	_sceVu0MulMatrix(mats, &DG_UnitMatrix, mats);
      
      vec.vx = -skel->rt_tx;
      vec.vy = -skel->rt_ty;
      vec.vz = -skel->rt_tz;
      _sceVu0ApplyMatrix((FVECTOR *)mats->m[3], mats, &vec);
    }
}


static void setup_camera(Work * work, int move)
{
  FVECTOR tmp;
  
  /* 現在の姿勢における頭の位置を得る */
  if(move)
    {
#ifdef DEBUG_MODE
      if(_DEBUG_face_camera)
	if(!work->side) printf("Camera move (side %d)\n", work->side);
#endif
      get_joints_vector(&work->head_pos, work->face.evmobj, HUMAN21_ATAMA);
    }
  else if(work->trace_camera)
    {
      /* 現在の頭の位置のZ位置と、以前に取得した頭の z 位置との差が大きければ、
	 カメラ位置を設定しなおす。 */
      get_joints_vector(&tmp, work->face.evmobj, HUMAN21_ATAMA);
      if((tmp.vz - work->head_pos.vz) >= 60.0F)
	{
	  work->head_pos = tmp;
	  /* カメラを動かすので、フォーカスブラーをかける。 */
	  /*
	    nfocusSwitch(work->nfocus_work, 1);
	    nfocusSetFocus(work->nfocus_work, 0, 0,
	    work->zfar - 100.0F, work->zfar);
	  */
	}
    }

  
  /* 計算済のカメラ位置と注視点を、頭の位置まで平行移動する */
  work->eye_target.vx += work->head_pos.vx;
  work->eye_target.vy += work->head_pos.vy;
  work->eye_target.vz += work->head_pos.vz;
  work->eye_target.vw *= work->head_pos.vw;
  
  /* 目標カメラ位置 */
  work->camera_pos.vx += work->head_pos.vx;
  work->camera_pos.vy += work->head_pos.vy;
  work->camera_pos.vz += work->head_pos.vz;
  work->camera_pos.vw *= work->head_pos.vw;

  if(move || !work->trace_camera) work->now_camera_pos = work->camera_pos;

  /* そのフレームのカメラ位置 */
  work->now_camera_pos.vx = work->now_camera_pos.vx +
    (work->camera_pos.vx - work->now_camera_pos.vx) / work->spin_delay;

  work->now_camera_pos.vy = work->now_camera_pos.vy +
    (work->camera_pos.vy - work->now_camera_pos.vy) / work->spin_delay;

  work->now_camera_pos.vz = work->now_camera_pos.vz +
    (work->camera_pos.vz - work->now_camera_pos.vz) / work->spin_delay;

  work->now_camera_pos.vw = work->now_camera_pos.vw +
    (work->camera_pos.vw - work->now_camera_pos.vw) / work->spin_delay;

  /* 目標点と現在カメラ位置の差の平均が 0.5F 以下であれば、
     フォーカスブラーは OFF */
  if(work->trace_camera)
    {
      float v;
      
      v = fabs(work->now_camera_pos.vx - work->camera_pos.vx);
      v += fabs(work->now_camera_pos.vy - work->camera_pos.vy);
      v += fabs(work->now_camera_pos.vz - work->camera_pos.vz);
      
      v /= 3.0F;
      // if(v < 0.5F) nfocusSwitch(work->nfocus_work, 0);
  }

#ifdef DEBUG_MODE
  if(_DEBUG_codec_face == 0)
#endif
    {
      FMATRIX mat;
      int x, y;

      /* カメラ位置を中心に,決定した注視点をパン角だけ回転させる */
      for(y = 0; y < 4; y++)
	for(x = 0; x < 4; x++)
	  mat.m[x][y] = (float)(x == y);

      _sceVu0RotMatrixY(&mat, &mat, work->now_camera_heading);
      _sceVu0RotMatrixX(&mat, &mat, work->now_camera_pitch);

#ifdef DEBUG_MODE
      if(_DEBUG_face_camera && !work->side) disp_mat(&mat);
#endif

      /* 注視点の相対位置を求める */
      _sceVu0SubVector(&(work->eye_target),
		       &(work->eye_target),
		       &(work->camera_pos));
      
      /* カメラからの相対注視点を, パン角のマトリクスで回転させる */
      _sceVu0ApplyMatrix(&(work->eye_target),
			 &mat, &(work->eye_target));

      /* 回転後のベクトルに, カメラ位置を加算 */
      _sceVu0AddVector(&(work->eye_target),
		       &(work->eye_target),
		       &(work->now_camera_pos));
    }

#ifdef DEBUG_MODE
  if(_DEBUG_codec_face == 3)   /* この場合,カメラと注視点は固定位置 */
    {
#ifdef PSX2
      work->eye_target = (FVECTOR){0.0F, 0.0F, 0.0F, 0.0F }; /* 原点 */
      work->now_camera_pos = (FVECTOR){0.0F, 0.0F, 4100.0F, 0.0F }; /* カメラ位置 */
#endif // PSX2
#ifdef KP_XBOX
      work->eye_target.vx = 0.0F;
      work->eye_target.vy = 0.0F;
      work->eye_target.vz = 0.0F;
      work->eye_target.vw = 0.0F;
      work->now_camera_pos.vx = 0.0F;
      work->now_camera_pos.vy = 0.0F;
      work->now_camera_pos.vz = 4100.0F;
      work->now_camera_pos.vw = 0.0F;
#endif
  }
  
#endif


  BP_Camera_SetCodecCamera();  //BP_CAMERA - set active camera for wide screen tweak system
  DG_SetCamera2(&DG_Chanls[work->side + BASE],
		&work->now_camera_pos, &work->eye_target,
		work->now_zoom + work->zoom_adjust);
  
#ifdef DEBUG_MODE
  if(_DEBUG_face_camera)
    if(!work->side)
      {
	int i;
	
	// printf("camera <%d>\n", work->side);
	printf("heading: %8.3f\n", work->now_camera_heading);
	printf("pitch: %8.3f\n", work->now_camera_pitch);
	printf("HEAD: ");   disp_vec(&work->head_pos);
	printf("camera: "); disp_vec(&work->camera_pos);
	printf("target: "); disp_vec(&work->eye_target);
	// disp_mat(&DG_Chanls[work->side + BASE].eye_pers);
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
  SPR_OBJ * guide;
  SPR_OBJ * root;
  SPR_POS pos;
  SPR_RECT rect;
  float fw, fh;
  int i;


  return;

  side = work->side;
  chanl = side + BASE;

  
  /* 表示領域ガイドとなるオブジェクトのポインタを得る */
  guide = codecGetObject((!side) ? CDC_FACE_RECT_L : CDC_FACE_RECT_R);
  root  = codecGetObject(CDC_LAYOUT_ROOT);


  /*
    width  = (int)((float)env_list[side].w * w_rate);
    height = (int)((float)env_list[side].h * h_rate);
  */
  ASSERT(guide->head.id == SP_BOX);  /* 表示ガイドは box である必要がある */

  width = guide->box.rect.end.x - guide->box.rect.begin.x;
  height = guide->box.rect.end.y - guide->box.rect.begin.y;

  pos.x = guide->box.rect.begin.x + root->empty.pos.x;
  pos.y = guide->box.rect.begin.y + root->empty.pos.y;

  SPR_SetPosSprite(work->over_filter, &pos);
  SPR_SetSizeSprite(work->over_filter, width, height);
  
  fw = (float)DRAW_WIDTH / 512.0F;
  fh = (float)DRAW_HEIGHT / 384.0F;
  
  for(i = 0; i < 2; i++)
    {
      rect.begin.x = 0.0F + (float)i * fw;
      rect.begin.y = 0.0F + (float)i * fh;
      rect.end.x = width  - (float)(1 + i) * fw;
      rect.end.y = height - (float)(1 + i) * fh;
      SPR_SetPosBox(work->mask_frame[i], &rect);
      SPR_SetColorBox(work->mask_frame[i], 40, 110, 90, 128);
      work->mask_frame[i]->head.flags |= SPR_FLAG_PRIV;
      SPR_SHOW(work->mask_frame[i]);
    }

#ifndef NO_GREEN_FILTER
  SPR_SHOW(work->over_filter);
#endif /* NO_GREEN_FILTER */
}

static void set_disp_env(Work * work, int disp)
{
  int side;
  int chanl;
  int width, height;
  int x, y;
  SPR_OBJ * rect;
  SPR_OBJ * root;

  side = work->side;
  chanl = side + BASE;

  /* 表示領域ガイドとなるオブジェクトのポインタを得る */
  rect = codecGetObject((!side) ? CDC_FACE_RECT_L : CDC_FACE_RECT_R);
  root = codecGetObject(CDC_LAYOUT_ROOT);

  /*
    width  = (int)((float)env_list[side].w * w_rate);
    height = (int)((float)env_list[side].h * h_rate);
  */
  ASSERT(rect->head.id == SP_BOX);  /* 表示ガイドは box である必要がある */

  width = rect->box.rect.end.x - rect->box.rect.begin.x;
  height = rect->box.rect.end.y - rect->box.rect.begin.y;

  x = (rect->box.rect.begin.x + rect->box.rect.end.x) / 2.0F +
    root->empty.pos.x;
  y = (rect->box.rect.begin.y + rect->box.rect.end.y) / 2.0F +
    root->empty.pos.y;
#if 1 // BP_Render #ifdef KP_XBOX
  x = (int)((float)x * (float)DRAW_WIDTH / 512.0F);
  width = (int)((float)width * (float)DRAW_WIDTH / 512.0F);
#endif
  y = (int)((float)y * (float)DRAW_HEIGHT / 384.0F);
  height = (int)((float)height * (float)DRAW_HEIGHT / 384.0F);

  if(!disp)
    {
      DG_Chanls[chanl].flag = 0;
      SPR_HIDE(work->over_filter);
      return;
    }
  if(height == 0) height = 1;  /* 表示される際は、最低でも 1ラスタは表示する */

  /* 表示領域の設定 */
  DG_SetDrawEnv(&DG_Chanls[chanl],
		(x - DRAW_WIDTH / 2), (y - DRAW_HEIGHT / 2), width, height);

  /* カメラの設定 */
  BP_Camera_SetCodecCamera();  //BP_CAMERA - set active camera for wide screen tweak system
  DG_SetCamera2(&DG_Chanls[chanl],
		&work->camera_pos, &work->eye_target, work->now_zoom);
  
  /* 表示状態に移行 */
  DG_Chanls[chanl].bg_clear_flag = 0;  /* 毎フレーム背景を消去 */

  DG_Chanls[chanl].flag = 1;
}

static void disp_sequence(Work * work)
{
  int side;

  side = work->side;

  switch(work->action_step)
    {
    case STEP_CLOSE:    /* 閉じ状態(非表示)        */
      set_disp_env(work, 0);
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

	set_disp_env(work, 1);
	if((w_rate == 1.0F) && (h_rate == 1.0F))
	  work->action_step = STEP_FOCUS;
	work->action_cnt += TIME_BASE;
      }
      break;
    case STEP_FOCUS:    /* ズーム/フォーカシング中 */
      set_filter_size(work, 1.0F, 1.0F);
      set_disp_env(work, 1);
#ifdef EFFECT_FOCUS
      if(!zoom_proc(work)) work->action_step = STEP_OPENED;
#else  /* EFFECT_FOCUS */
      work->action_step = STEP_OPENED;
#endif /* EFFECT_FOCUS */
      break;
    case STEP_OPENED:   /* 表示完了                */
      set_filter_size(work, 1.0F, 1.0F);
      set_disp_env(work, 1);
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
	set_disp_env(work, 1);
	if((w_rate == 0.0F) && (h_rate == 0.0F))
	  work->action_step = STEP_CLOSE;
	work->action_cnt -= TIME_BASE;
      }
      break;
    }
}

void codecCloseFace(void * workp)
{
  Work * work = workp;
  if(NULL == workp) return;
  init_focus(work);
  work->action_step = STEP_CLOSING;
}

int codecFaceIsClosed(void * workp)
{
  Work * work = workp;
  if(NULL == workp) return 1;
  if(work->action_step == STEP_CLOSE) return 1;
  return 0;
}

/*
 * オブジェクト用モーション割り当て
 */
static void set_object_joints(OBJECT * obj, int n_joints, FVECTOR * vecarr)
{
  int min;
  int i;
  FVECTOR * rot;
  FVECTOR * trans;
  FMATRIX mat;
  
  /* モーションとモデルの関節数で、少ないほうに合わせる */
   min = (obj->objs->def->n_x_models < n_joints)
    ? obj->objs->def->n_x_models : n_joints;

  for(i = 1; i <= min; i++)
    {
      rot = vecarr + i * 2;
      trans = vecarr + i * 2 + 1;

      MT_QuatToMat(&mat, rot);
      mat.m[3][0] += trans->vx + obj->objs->trans.vx;
      mat.m[3][1] += trans->vy + obj->objs->trans.vy;
      mat.m[3][2] += trans->vz + obj->objs->trans.vz;
      _sceVu0MulMatrix(&mat, &(obj->objs->world), &mat);
      obj->objs->world = mat;
      obj->objs->objs[i - 1].world = mat;
    }
}


static void fips_size_parts(Work * work)
{
  SPR_OBJ * guide;
  float px, py;  /* 画面上の物理位置   */
  float pw, ph;  /* 画面上の物理サイズ */
  float home_x, home_y;


  /* 画面左上の座標値 */
  home_x = 2048.0F - (float)(DRAW_WIDTH / 2);
  home_y = 2048.0F - (float)(DRAW_HEIGHT / 2);

  /* 位置, サイズを求める */
  guide = codecGetObject((!work->side) ? CDC_FACE_RECT_L : CDC_FACE_RECT_R);
#ifdef DEBUG_MODE
  if(_DEBUG_face_guide)
    {
      guide->box.col.r = 128;
      guide->box.col.g = 128;
      guide->box.col.b = 128;
      guide->box.col.a = 128;
    }
#endif
  px = guide->box.rect.begin.x;
  py = guide->box.rect.begin.y;

  pw = guide->box.rect.end.x - px;
  ph = guide->box.rect.end.y - py;

  /* 求められた位置とサイズを上にかけるフィルタに設定する */
  {
    SPR_POS pos;
    SPR_RECT rect;
    int i;

    pos.x = px;
    pos.y = py;
    SPR_SetPosSprite(work->over_filter, &pos);     /* 位置 */
    SPR_SetSizeSprite(work->over_filter, pw, ph);  /* サイズ */

    /* 大きさがあれば、縁どりの box を描画し、可視にする */
    for(i = 0; i < 2; i++)
      {
	rect.begin.x = 0.0F + (float)i;
	rect.begin.y = 0.0F + (float)i;
	rect.end.x = pw - (float)(1 + i);
	rect.end.y = ph - (float)(1 + i);
	SPR_SetPosBox(work->mask_frame[i], &rect);
      }
    
    /* 表示領域に大きさがあれば、可視状態にする */
    if((pw >= 0.0F) && (ph >= 0.0F) && (work->disp))
      {
	SPR_SHOW(work->over_filter);
	SPR_SHOW(work->mask_frame[0]);
	SPR_SHOW(work->mask_frame[1]);
      }
    else
      {
	SPR_HIDE(work->over_filter);
	SPR_HIDE(work->mask_frame[0]);
	SPR_HIDE(work->mask_frame[1]);
      }
  }
  
  /* 位置, サイズを、画面上の物理的な数値に変換する */
  px = px * DRAW_WIDTH / SPR_SCRN_WIDTH + home_x;
  py = py * DRAW_HEIGHT / SPR_SCRN_HEIGHT + home_y;

  pw = pw * DRAW_WIDTH / SPR_SCRN_WIDTH;
  ph = ph * DRAW_HEIGHT / SPR_SCRN_HEIGHT;

  /*
   * 背景板
   */
  /* 背景板は、各顔が表示されているチャネル全体に描画されているため、
     座標値は別扱いになる。 */
#if 0 // BP_Render #ifdef PSX2  
  work->plate[DG_Clock].gif.data.xyz0.data =
    SCE_GS_SET_XYZ((2048 - DG_Chanls[work->side + 2].width / 2) * 16,
		   (2048 - DG_Chanls[work->side + 2].height / 2) * 16, 1);
  work->plate[DG_Clock].gif.data.xyz1.data =
    SCE_GS_SET_XYZ((2048 + DG_Chanls[work->side + 2].width / 2) * 16,
		   (2048 + DG_Chanls[work->side + 2].height / 2) * 16, 1);
#else //XBOX
	{
		void			*prim ;
		unsigned int	col;
		float chanlw, chanlh;
		
		col = FACEBG_R | ( FACEBG_G << 8 ) | ( FACEBG_B << 16 ) | ( 128 << 24 ); 
		prim = work->plate_dmapack->autopacket ;
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 1, 0, 1, 128) );
 		chanlw = (float)(DG_Chanls[work->side + 2].width);
		chanlh = (float)(DG_Chanls[work->side + 2].height);
		prim = DG_SetDmapackSetZ( prim, 0.0f );
		prim = DG_SetDmapackBox( prim, 
								 0.0f, 0.0f, chanlw, chanlh,
								 DG_MakeDmaPackColorFromInt(col) );
		prim = DG_SetDmapackEnd( prim );
	}	
#endif
}


/*
 * オタコンの眼鏡を手の動きに連動させる
 */
static void otacon_hand(Work * work, OptionObject * obj)
{
  FMATRIX * hand_mat;
  FMATRIX * gl_mat;
  FMATRIX   tmp_mat;
  FVECTOR   otchand = {51.0F, -156.0F, 5.0F, 1.0F}; /* 右手首押えポイント   */
  FVECTOR   glasses = { 0.0F, 40.0F, 119.0F, 1.0F}; /* 眼鏡押えられポイント */
  FVECTOR   gl_core = { 0.0F, 55.0F,  19.0F, 1.0F}; /* 眼鏡回転中心         */
  FVECTOR   tmp;

  hand_mat = &(work->face.objs->objs[HUMAN21_MIGI_TE].world);/* 指マトリクス */
  gl_mat   = &( obj->obj.objs->objs[0].world);             /* 眼鏡マトリクス */
  
  /* 眼鏡を抑えるポイントの座標値に、右手のマトリクスをかけると、
     現在の右手の先のワールド位置が出て来る */
  _sceVu0ApplyMatrix(&otchand, hand_mat, &otchand);
  _sceVu0SubVector(&tmp, &otchand, gl_mat->m[3]);

  tmp_mat = *gl_mat;
  tmp_mat.m[3][0] = 0.0F;
  tmp_mat.m[3][1] = 0.0F;
  tmp_mat.m[3][2] = 0.0F;
  
  /* 右手の指先の位置を、眼鏡基準に変換する */
  _sceVu0InversMatrix(&tmp_mat, &tmp_mat);  /* 眼鏡の逆行列を求める */

  /* 右手の指先が, 眼鏡の原点に対してどの位置にあるかを確認する */
  _sceVu0ApplyMatrix(&otchand, &tmp_mat, &tmp);

  /* 指先と押えられるポイントが,どの程度近いかを計算する */
  _sceVu0SubVector(&tmp, &otchand, &glasses);

  /* 一定以上離れていれば, 押え中ではないとみなし何もしない */
  if(fabs(tmp.vy) < -0.5F) return; /* 手が下すぎる                   */
  if(fabs(tmp.vz) > 20.0F) return; /* 指先が眼鏡から離れすぎ         */
  if(fabs(tmp.vx) > 15.0F) return; /* 指先が眼鏡中央から横にずれすぎ */
  
  /* 押えの条件を満たしていれば,眼鏡の回転角を指先の位置に追従させる */
  
  if(tmp.vy < 0.0F) tmp.vy = 0.0F;    /* 眼鏡より下の場合はまだ動かない */

  
  {
    float r = glasses.vz - gl_core.vz;
    float rot;
    int x, y;
    FMATRIX mat;
    FVECTOR vec = { 0.0F, -55.0F, -19.0F, 1.0F };

    rot = atan(-tmp.vy / r) * 2;
    for(y = 0; y < 4; y++)
      for(x = 0; x < 4; x++)
	mat.m[x][y] = (float)(x == y);

    _sceVu0TransMatrix(&mat, &mat, &vec);
    _sceVu0RotMatrixX(&mat, &mat, rot);
    _sceVu0TransMatrix(&mat, &mat, &gl_core);
    _sceVu0MulMatrix(gl_mat, gl_mat, &mat);
  }  
}

/*
 * 眼球の関節を,顔の中心に対し 45度上向きにする
 */
#define EYE_SKEL_L 26
#define EYE_SKEL_R 27

static void BugEyesSet(Work * work)
{
  DG_EVMOBJ * evmobj = work->face.evmobj;
  FMATRIX   * skel_mats;
  FMATRIX   * mats;
  FMATRIX     mat;
  EVM_SKEL  * skel;
  FVECTOR   * rot;
  FVECTOR     vec;
  int i, x, y;
  
  skel_mats = evmobj->matrix[ evmobj->use_buffer ];
  skel = evmobj->def->skeleton + EYE_SKEL_L;
  vec.vw = 1.0F;
  for(i = 0; i < 2; i++)
    {
      mats = &skel_mats[ i + EYE_SKEL_L ];
      /* X軸回転45°、トランスレーション無し */
      for(y = 0; y < 4; y++)
	for(x = 0; x < 4; x++)
	  mats->m[x][y] = (float)(x == y);
      _sceVu0RotMatrixX(mats, mats, F_PI / 4.0F);

      mats->m[3][0] = skel->rt_tx;
      mats->m[3][1] = skel->rt_ty;
      mats->m[3][2] = skel->rt_tz;
      _sceVu0MulMatrix(mats, &skel_mats[ skel->parent ], mats);

      vec.vx = -skel->rt_tx;
      vec.vy = -skel->rt_ty;
      vec.vz = -skel->rt_tz;
      _sceVu0ApplyMatrix((FVECTOR *)mats->m[3], mats, &vec);
    }
}

/*****************************************************************************/
/*****************************************************************************/
static void Act(Work * work)
{
  int move = 1;
  int body_move = 0;

//  DmSetDataBreakpoint( &( work->over_filter ), DMBREAK_NONE, 4 );
  /*
    現在の顔表示領域に,以下の装飾オブジェクトのサイズを合わせる。
    
    背景板
    フィルタ
    走査線
  */
  /* 表示 / 非表示の切替えを行う */
  if(work->disp != work->nxt_disp)
    {
      work->disp = work->nxt_disp;   /* 表示状態を最新の状態に更新 */
      if(!work->disp)
	{
	  DBG("BackFrame Hide!!\n");
	  work->action_step = (!work->disp_fade) ? STEP_CLOSING : STEP_CLOSE;
	  // SPR_HIDE(work->over_filter);
	}
      else
	{
	  DBG("BackFrame Show!!\n");
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
  fips_size_parts(work);
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
  work->mtn_stream = 0;
  /* モーションのストリーミング再生 */
  {
    DEMO_MOTION * mtn;
    int start;

#ifndef NO_BODY_MOTION
    if(work->pre_motion != NULL) move = 0;

    /* 顔モーションを割り当てる前に、身体モーションを割り当てる */
    if(NULL != (mtn = GM_StreamGetMotion(work->body_id)))
      {
	DBG("%d: body motion\n", work->side);

	/* モーションがある場合はそのモーションを再生する */
   DG_Arm_SwitchEvmBuffer(work->face.evmobj);
	/* 最後に指定したストリーミングモーション値を保持する */
	save_body_motion(work, 0, 21, mtn->motion);
	set_joints(work->face.evmobj, 0, mtn->n_joints, mtn->motion);
	syncro_obj(work);
	body_move = 1;
	work->mtn_stream = 1;
	/* ストリーミングモーション再生中は注視点を動かさない */
	// if(work->body_move) move = 0;
	work->body_move |= 0x0001;
	work->pre_motion = mtn;
	
	if(work->cnt_face_mtn >= 2)
	  GV_CallChildSignalFunc(work, HAIR_SIGNAL_CALC_FLAG, 1);
	else
	  work->cnt_face_mtn++;
      }
#ifdef FACE_DEFAULT_MOTION
    else
      {
	/* モーションが無かった場合は、デフォルトモーションの再生を行う */
	// work->face.evmobj->use_buffer = 1 - work->face.evmobj->use_buffer;
	if(work->pre_motion != NULL)
	  {
	    DBG("%:d no motion (continue)\n", work->side);
	    /* NULL ではないことが重要であるためポインタの値は特に意味は無い */
	    work->pre_motion = (void *)work;
	    work->motion_fix = 0;

	    /* 最後の全身モーションを再度割り当てる */
	    hold_body_motion(work);
	    move = 0;
	  }
	else
	  {
	    DBG("%d: default motion\n", work->side);
	    // EVMOBJ のバッファバッファは、GM_ActObject2 が行うので
	    // こちらでは行わない
	    play_default_motion(work);
	    work->pre_motion = NULL;
	    move = 1;
	  }
	
	
	if(work->cnt_face_mtn >= 2)
	  GV_CallChildSignalFunc(work, HAIR_SIGNAL_CALC_FLAG, 1);
	else
	  work->cnt_face_mtn++;

	// work->body_move = 1;
      }
#endif  /* FACE_DEFAULT_MOTION */
#endif  /* NO_BODY_MOTION */

#ifndef NO_FACE_MOTION
    /* 現在のフレームの顔モーションを得て、
       身体モーションを適用しおわった直後の EVM モデルに上書きする */
    if(work->face.evmobj->n_skeleton >= 53)
      {
	if(NULL != (mtn = GM_StreamGetMotion(work->speaker_id)))
	  {
	    DBG("%d: stream face motion\n", work->side);
	    /* 全関節の値を設定する */
	    start = (mtn->n_joints == 32) ? 21 : 0;
	    set_joints(work->face.evmobj, start, mtn->n_joints, mtn->motion);
	    /* 最後に設定した顔関節値として保存する */
	    save_face(work, start, mtn->n_joints, mtn->motion);
	    work->pre_face = 1;
	    work->mtn_stream |= 0x0002;
	  }
	else if(!work->pre_face || work->canceled)
	  {
	    DBG("%d: default face motion\n", work->side);
	    /* 条件をキャンセルされたか一度でもストリーミングモーションの
	       顔を再生したかに変更。 */

	    /* モーションが無い場合は、顔の全関節をニュートラルにする。
	     * ※音声をキャンセルした際に、口が開きっぱなしになるのを防ぐため。
	     *   ただし、53関節モデルに限定する。
	     *   忍者のような口を動かさない 21関節モデルでは何もしない。*/
	    set_joints(work->face.evmobj, 21, 32, work->nutral);
	    /* 最後に設定した顔関節値として保存する */
	    save_face(work, 21, 32, work->nutral);
	  }
	else
	  {
	    DBG("%d: fix face motion\n", work->side);
	    /* 直前のフレームの顔関節を,今回も継承する。 */
	    cont_joints(work, 21, 32);
	    /* 関節値自体は変わらないので,セーブしない */
	    body_move = 1;  /* 動いたことにして、目パチは禁止する */
	  }
	/* 白目フラグが有効なら白目を剥かせる */
	if(work->bug_eyes) BugEyesSet(work);
      }
#endif  /* NO_FACE_MOTION */

    /*
     * オブジェクトモーション割り当て
     */
    {
      OptionObject * obj;
      SVECTOR rot;
      FVECTOR mov;

      /* 付属品全てにモーションの適用を行う */
      for(obj = work->begin; obj != NULL; obj = obj->next)
	{
	  {
	    /* とりあえず、頭のマトリクスを割り当てる */
	    FMATRIX mat;

	    obj->obj.objs->objs[0].world =
	      obj->obj.objs->world =
	      work->face.objs->objs[ obj->skel ].world;
	  }
	  if(NULL != (mtn = GM_StreamGetMotion(obj->name)))
	    {
	      /* オブジェクト用のストリーミングモーションがある場合 */
	      set_object_joints(&(obj->obj), mtn->n_joints, mtn->motion);
	    }
	  else
	    if(obj->name == (0x004ab569 /* "オタコン" */ + 2))
	      {
		/* オブジェクトの ID がオタコンの付属品であれば、
		   眼鏡とみなし、右手の指と連動させる */
		otacon_hand(work, obj);
	      }
	}
    }

    /* 身体のモーションが無ければ, 自動目パチを入れる */
    if(!body_move) EyeAnimation(work);

    /* 胸像 EVM の world の値を出力する */
    // disp_mat(&work->face.evmobj->world);
  }

  DBG("mtn_stream(%d) = %04x\n", work->side, work->mtn_stream);

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
    
#ifdef KP_XBOX
    _sceVu0MulMatrix(&DG_Chanls[work->side + BASE].eye_xpers,
		     &DG_Chanls[work->side + BASE].eye_xpers, mat);
    //BP TED - this looks like a bugfix that should not have been restricted to Windows version
    BP_RENDER_TODO_BREAK;
#ifdef KP_WINDOWS
    _sceVu0MulMatrix(&DG_Chanls[work->side + BASE].eye_inv,
		     &DG_Chanls[work->side + BASE].eye_inv, mat);
#endif
#endif
    _sceVu0MulMatrix(&DG_Chanls[work->side + BASE].eye_pers,
		     &DG_Chanls[work->side + BASE].eye_pers, mat);
  }
#endif /* EFFECT_HANDYCAM */

//  DmSetDataBreakpoint( &( work->over_filter ), DMBREAK_WRITE, 4 );
  
}

static void Die(Work * work)
{
  int i;

  if(now_work[work->side] == work) now_work[work->side] = NULL;
  /*
    if(work->effect_works != NULL) GV_DestroyActor(work->effect_works);
    if(work->hsync_work != NULL) GV_DestroyActor(work->hsync_work);
  */

  free_all_object(work);

#ifdef FACE_DEFAULT_MOTION
  destroy_default_motion(work);
#endif /* FACE_DEFAULT_MOTION */
  if(work->over_filter != NULL) SPR_Destroy_2D_Object(work->over_filter);
  if(work->plate_dmapack != NULL)
    {
      DG_DequeueDmapack(work->plate_dmapack);
      DG_FreeDmapack(work->plate_dmapack);
      DBG("call codecFree()\n");
      codecDelayedFree(work->plate);
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


/*
 * EVM モデルを設定する
 */
static int setup_evm(Work *work, int name, int model_id, int speaker_id,
		     int no_reset)
{
  printf(" ===================== FACE EVM SETUP ======================\n");
  work->body_move = 0;
  work->bug_eyes = 0;
  work->trace_camera = 0;
  printf("dummy_bone = %d\n", work->dummy_bone);
  CDC_InitObject(&(work->face), work->dummy_bone,
		 DG_FLAG_INVISIBLE, work->side + 2);
  CDC_ConfigObjectEvm(&(work->face), model_id, 0, work->side + BASE);

  /* evmobj に対し、表示チャネルの設定を行う */
  /* 左側はチャネル2, 右側はチャネル3 で扱う */
  work->face.evmobj->chanl = work->side + BASE;
  work->face.evmobj->flag  = (!work->side) ? MODEL_FLAG_0 : MODEL_FLAG_1;

  if(!no_reset) face_lighting(work);  /* 光源初期化 */
  GM_ConfigObjectLight(&(work->face), work->light_matrix);

  work->name = name;
  work->speaker_id = speaker_id;      /* 顔モーション ID   */
  work->body_id    = speaker_id + 1;  /* 身体モーション ID */
  work->pre_face   = 0;

  DBG("speaker ID = %d\n", work->speaker_id);

  disp_mat(&(work->face.evmobj->world));

  /* 基本注視点を設定 */
  get_joints_vector(&work->eye_target, work->face.evmobj, HUMAN21_ATAMA);

  work->close_eyes_motion = NULL;
  work->close_eyes_cnt    = 0;
  work->close_eyes_wait   = 0;
  work->close_eyes_rate   = 0.0F;
  work->close_eyes_step   = EYES_STAT_OPEN;

  work->pre_motion = NULL;

  return 0;
}

/* デバッグのため,エフェクトを全てカットする */

static int GetResources(Work *work, int name,
			int dummy_bone, int model_id, int speaker_id)
{
  int i, ix, iy;
  float x, y, w, h;

  /* 付属品リストを初期化する */
  work->begin = NULL;
  work->end = NULL;

  /* キャンセルしたことがあるフラグは、最初の一回のみ初期化 */
  work->canceled = 0;

  /* カメラ操作は、デフォルトでは許可 */
  work->camera_ctrl = 1;

#ifdef DEBUG_MODE
  work->light_angle = 0.0F;
  work->ring_angle = 0.0F;
#endif /* DEBUG_MODE */

  work->camera_heading = 0.0F;
  work->camera_pitch   = 0.0F;
  work->target_heading = 0.0F;
  work->target_pitch   = 0.0F;

  work->now_camera_heading = work->camera_heading;
  work->now_camera_pitch   = work->camera_pitch;

  for(i = 0; i < HAIR_MAX; i++) work->hair_work[i] = NULL;
  work->hair_num = 0;

  work->dummy_bone = dummy_bone;

  /* カメラ位置移動マトリクスを初期化 */
  for(iy = 0; iy < 4; iy++)
    for(ix = 0; ix < 4; ix++)
      work->camera_work.m[ix][iy] = (float)(iy == ix);

  /* カメラ操作のディレイタイム */
  work->spin_delay = 75.0F / (float)TIME_BASE;
  work->zoom_delay = 300.0F / (float)TIME_BASE;

  /* カメラ位置初期化 */
  work->zfar = 1000.0F;
  work->zoom = 11.0F;
  work->heading = 0.0F;
  work->pitch   = 0.0F;
  work->pan     = 0.0F;
  work->gain    = 0.0F;

  /* リミッタ初期化 */
  work->heading_right = F_PI / 8.0F;  /*  22.5° */
  work->heading_left  = F_PI / 8.0F;  /* -22.5° */
  work->pitch_lower   = F_PI / 180.0F; /*  1° */
  work->pitch_upper   = F_PI / 8.0F;  /* -22.5° */
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

  /*
    w = env_list[work->side].w * 512.0F / DRAW_WIDTH;
    h = env_list[work->side].h * 384.0F / DRAW_HEIGHT;
    x = (env_list[work->side].x - env_list[work->side].w / 2) * 512.0F / DRAW_WIDTH;
    y = (env_list[work->side].y - env_list[work->side].h / 2) * 384.0F / DRAW_HEIGHT;
  */

  /* 下地用プレートを用意 */
  setup_bg_plate(work);

  /* かぶせ用フィルタを作成 */
  work->over_filter = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
  ASSERT(work->over_filter != NULL);
  {
    SPR_POS pos;
    SPR_RECT rect;
    float fw, fh;

    /*
     * 上にかけるフィルタは,この段階では位置やサイズを決定しない
     * (ガイドの box に従うため)
     */
    work->over_filter->head.pri = 0;
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
	/*
	  rect.begin.x = 0.0F + (float)i * fw;
	  rect.begin.y = 0.0F + (float)i * fh;
	  rect.end.x = w - (float)(1 + i) * fw;
	  rect.end.y = h - (float)(1 + i) * fh;
	  SPR_SetPosBox(work->mask_frame[i], &rect);
	*/
	SPR_SetColorBox(work->mask_frame[i], 40, 110, 90, 128);
	work->mask_frame[i]->head.flags |= SPR_FLAG_PRIV;
	SPR_HIDE(work->mask_frame[i]);
      }
    
  }

  /* 顔画面にかけるエフェクトを担当する Actor の登録 */
#ifdef EFFECT_CRT
  DBG("FACE CRT Effect.\n");
  //BP TED - this looks like a bugfix that should not have been restricted to Windows version
  BP_CODEC_RENDER_TODO_BREAK;
#ifndef KP_WINDOWS
  GV_SetActorChild(work, NewCodecEffectCRT(work->side, 0, 0, FACE_WIDTH, FACE_HEIGHT));
#else
  GV_SetActorChild(work, NewCodecEffectCRT(work->side,
				(DRAW_WIDTH  - 640) >> 1,	// X-BOX版との誤差修正
				(DRAW_HEIGHT - 448) >> 1,	// X-BOX版との誤差修正
				FACE_WIDTH, FACE_HEIGHT));
#endif

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

#ifdef EFFECT_STATIC_NOISE
  GV_SetActorChild(work, NewCodecStaticNoise(work->side));
#endif


  setup_evm(work, name, model_id, speaker_id, 0);
  work->motion_fix = 0;
  work->mtn_camera_cnt = 0;
  return 0;
}

/*****************************************************************************/

/* 指定されたサイドの顔モデルを変更する */
void codecFaceChange(int side, int model_id, int speaker_id)
{
  Work *work;
  int i;

  /* 指定されたサイドにまだ Work が作られていない場合は何もしない */
  if(NULL == (work = now_work[side]))
    {
      HANGUP();  // これは異常
      return;
    }

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
      for(i = 0; i < work->hair_num; i++)
	if(NULL != work->hair_work[i])
	  {
	    GV_DestroyOtherActor(work->hair_work[i]);
	    work->hair_work[i] = NULL;
	  }
      work->hair_num = 0;

      /* 顔のモデルが定義されていれば、破棄する */
      CDC_FreeObject(&(work->face));

#ifdef FACE_DEFAULT_MOTION
      destroy_default_motion(work);
#endif /* FACE_DEFAULT_MOTION */
    }

  /* 付属品を全て開放する */
  free_all_object(work);

  /* EVM オブジェクトを設定しなおす */
  setup_evm(work, 0, model_id, speaker_id, 1);
}

/*****************************************************************************/

void * NewCodecFace(int name, int dummy_bone,
		    int side, int model_id, int speaker_id)
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
      ASSERT(0);
      return NULL;
    }
  work->disp = 0;
  work->nxt_disp = 0;
  work->disp_fade = 0;

  /* 現在の指定サイドの Work として登録 */
  now_work[side] = work;
  return work;
}
