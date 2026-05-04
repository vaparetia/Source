//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
#include "libfs.h"
//BP
//----------------------------------------------------------------------------
/*
        face.c
	表情、目パチ、口パクの各顔モーションを独立して動かし、合成する。
	口パクドライバに対応。

	2000/07/18 Y.Kira
	$Id: face.c,v 1.1.1.3 2002/11/19 11:43:54 Yoshizawa1 Exp $

*/
/*

シナリオ呼び出しインターフェイス
chara 顔アニメ[NewFaceMotionSet] $s:name \
	-chara	$s:キャラクタ名 \
	-a $s:mtn_a \
	   $s:mtn_i \
	   $s:mtn_u \
	   $s:mtn_e \
	   $s:mtn_o \
	   $s:mtn_N \
	   $s:mtn_v \
	   $s:mtn_eyes

指定のキャラに、目パチ、口パクなどのモーションを定義し、
ストリーミングの音声に合わせて口パクができるようにします。


mesg 顔アニメ $s:名前 表情設定[0] $s:モーション名 $w:レート $w:補間時間
//  モーション名にはrmtファイルのファイル名を指定
//  レートは表情の強さを表す。0～1000の値を設定。
//     0 は完全なニュートラル状態、1000がモーションデータそのままの表情。
//  補間時間は、表情の変更にかける時間。1/300秒単位で指定。

mesg 顔アニメ $s:名前 表情ニュートラル[1] $w:補間時間
//  補間時間は、表情の変更にかける時間。1/300秒単位で指定。

mesg 顔アニメ $s:名前 口パク強度[2] $w:強度
// 口パクの大きさを指定。0～1000 を指定。1000 が最も動きとしては大きい。

mesg 顔アニメ $s:名前 強制目パチ[3] $w:目パチ回数
// 強制的に目パチを行わせる。指定された回数だけの目パチを行う。


※このキャラで使用するモーションファイルというのはモーション班に作成してもらった
　顔のみのモーションデータが含まれているｒｍｔファイル（無圧縮生モーションデータ）
　のことです。注意してください。


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include "BP_EndianSupport.h"

#include "lip_config.h"    /* コンパイルスイッチなどの集合 */

#ifdef  FACE_DEBUG
#define DBG(...)   printf(__VA_ARG_LIST);
#else
#define DBG(...)
#endif /* FACE_DEBUG */


#include "lip_id.h"
#include "eyes.h"

//#ifdef JAPANESE_BP_IGNORE()
#include        "lip_japanese.h"
//#endif

//#ifdef ENGLISH
#include        "lip_english.h"
//#endif

#include        "face_lip.h"

/* ---------------------------------------------------------------- */
/* 各種定義 */
extern int BP_BASE_TICK(void);
#define TIME_BASE	(BP_BASE_TICK())     /* NTSC での 1フレームに相当する時間 */


#define CLOSE_SPEED  10      /* 無音部分で口を閉じる最長の時間
				(音声時系列単位)                     */
#define MIN_STEP     5       /* 最短の音素長(音声時系列単位)         */


#define FACE_STATUS_MORFING  0x80    /* 補間中 */



/*
 * 以下の二つは現状では使用されていない。
 */
#define MIN_POWER    0.5     /* 最小パワー                           */
#define SILENT_POWER 1.0     /* 無音部のパワー                       */
#define MOUTH_RATE   0.5     /* 表情と口のブレンド率                 */

#ifdef SILENT_NO_CLOSE_MOUTH
#define SILENT_MOUTH_RATE 0.1
#endif /* SILENT_NO_CLOSE_MOUTH */
/* ---------------------------------------------------------------- */
/* 内部使用構造体 */

/* モーションデータ管理構造体 */
typedef struct {
	int			name ;
	int			count ;
	int			length ;
	int			joints ;
	FVECTOR		*move ;
	FVECTOR		*rots ;
	FVECTOR		*trans ;
} MTN_DATA ;

/* スクラッチパッドワーク定義 */
typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[64] ;
} ScrPadWork ;



/* ---------------------------------------------------------------- */
/* 口パク用モーションのセット */
typedef struct LipMotions {
  MTN_DATA      mtn_a;          /* /a/ をあらわすモーション         */
  MTN_DATA      mtn_i;          /* /i/ をあらわすモーション         */
  MTN_DATA      mtn_u;          /* /u/ をあらわすモーション         */
  MTN_DATA      mtn_e;          /* /e/ をあらわすモーション         */
  MTN_DATA      mtn_o;          /* /o/ をあらわすモーション         */
  MTN_DATA      mtn_N;          /* /N/ をあらわすモーション         */
  MTN_DATA      mtn_v;          /* /v/ をあらわすモーション         */
} LipMotions;


/* リアルタイムに作成される顔モーションの構造体 */
typedef struct {
  FVECTOR rots[35];
  FVECTOR trans[35];
} MTN_BLOCK;

/* モーション生成ガイド */
typedef struct {
  LipID motion_a;   /* ブレンドする口モーション A の ID */
  LipID motion_b;   /* ブレンドする口モーション B の ID */
  float rate;     /* ブレンド率                 */
} BlendInfo;


/* モーション遷移時系列構造体 */
typedef struct {
  float time;
  BlendInfo mtn;
} MorfTime;


/* ---------------------------------------------------------------- */
	/*
		ワーク構造体定義
	*/
typedef	struct	{
  GV_ACT_EX	actor ;
  int			name ;	/* キャラクタ名 */
  DG_EVMOBJ	*evmobj ;	/* ＥＶＭオブジェクト */

  /* 顔モーション関係 */
  int		interp_count ;	/* 補間時間 */
  MTN_DATA	mtn_data ;	/* ＭＴＮファイル情報＆再生フレーム情報 */

  /* -------------------------------------
   * 顔モーション関係
   * ------------------------------------- */
  LipMotions    lips;           /* 口モーションの集合体             */


  MTN_DATA      mtn_eyes;       /* 目閉じパターン                   */

  MTN_DATA      mtn_face[2];    /* 表情のモーションとして
				   再生するモーションデータ      */

  /* -------------------------------------
   * 口パク時系列制御関係
   * ------------------------------------- */
  int           mouth_playing;  /* 口パク中ステータス               */

  LipInfo     * lip_info;       /* 口パクドライバから渡される
				   口形状生成用情報                 */
  int           loaded;
  float         total_power;    /* 全体の動きにかける係数(0～1.0)   */


  /* -------------------------------------
   * 表情変更制御関係
   * ------------------------------------- */
  float         face_rate[2];      /* 表情のレート                     */
  int           mtn_face_loaded[2];/* 表情がロードされている場合は 1,
				      表情が無い場合は 0 */

  float         face_morf_rate;    /* 表情変更時の補間レート           */
  int           face_morf_time;    /* モーフィングの所要時間           */
  int           face_time_cnt;     /* モーフィング開始からの経過時間
				      ※将来的に、tick に置き換え      */

  int           face_status;       /* 表情補間のステータス
				      bit 0:  メイン側の表情
				      bit 7:  0: 表情固定 / 1: 補間中

				      最下位ビットは、現在メインとなっている
				      側の表情を指す。
				      補間中の場合は、メイン側に向かって
				      補間する。*/
  

  /* -------------------------------------
   * 目パチ関係
   * ------------------------------------- */
  int           eyes_flags;     /* 目パチの各種フラグ                      */
  int           eyes_t;         /* 最後に目パチさせた時刻からの経過時間    */
  int           eyes_counter;   /* 自動目パチの間隔カウンタ                */
  int           eyes_status;    /* 目パチのステータス                      */
  int           eyes_spd;       /* まぶたの動作カウンタ                    */
  int           eyes_times;     /* 連続させて目パチさせる回数              */
  float         eyes_rate;      /* 目パチのレート(0.0 = 全開 / 1.0 = 閉じ) */


  /* -------------------------------------
   * 最終的な顔モーション
   * ------------------------------------- */
  FVECTOR	face_skel_trans[ 35 ] ;	/* モーショントランスレーション配列 */
  FVECTOR	face_skel_rot[ 35 ] ;	/* モーションクォータニオン配列 */

  /* 視線制御関係 */
  int		eye_pos_type ;	/* 座標パラメータ基準タイプ */
  int		eye_interp_count ;	/* 視線制御補間時間 */
  FVECTOR	target_param ;		/* 座標パラメータ */
  FVECTOR	eye_target ;		/* 視線方向絶対座標 */
  FVECTOR	leye_rot ;			/* 左目回転量 */
  FVECTOR	reye_rot ;			/* 右目回転量 */
} Work ;

/*
 * 各言語対応の,動きへの変換テーブルを読み込む。
 */
struct SLipStruct {
   int  phone_id;
   MorfTime seq[3];
};

//#ifdef JAPANESE_BP_IGNORE()
#include "lip_japanese.c"
//#endif

//#ifdef ENGLISH
#include "lip_english.c"
//#endif

static inline struct SLipStruct *get_lip_motion()
{
   return BP_Area_JP() ? mtn_cnv_jpn : mtn_cnv_eng;
}

/* ---------------------------------------------------------------- */
	/*
		ローカル関数群
	*/
/* ＭＴＮデータのセットアップ */
static void SetMtnMotionData( MTN_DATA *mtn_data, int name_id )
{
  RMT_FILE_HEADER * mtn_file_header ;
  int               size ;

  /* ロード済みデータから該当データを検索 */
  mtn_file_header = GV_GetCache( GV_CacheID( name_id, 'r' ) );	/* 拡張子は.r??とする */
  if ( mtn_file_header != NULL ){
     RMT_VerifyEndianSwap( mtn_file_header );

    /* データが見つかった場合、管理構造体を初期化する */
    mtn_data->length = mtn_file_header->motion_length ;
    mtn_data->joints = mtn_file_header->motion_joints ;
    size = 0 ;
    mtn_data->move = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
    size = mtn_file_header->move_size ;
    mtn_data->rots = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
    size += mtn_file_header->rots_size ;
    mtn_data->trans = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
    size += mtn_file_header->trans_size ;
    mtn_data->name = name_id ;
    mtn_data->count = 1 ;
  } else {
    /* 失敗 */
    printf("rmt file open error !!\n");
    mtn_data->name = 0 ;
  }
}

/* ＥＶＭオブジェクトの一部に直接モーションを設定 */
static void EvmActMotionImmediate( DG_EVMOBJ *evmobj, int first, int n_joints, FVECTOR *skel_trans, FVECTOR *skel_rot )
{
  int			i ;
  ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
  FMATRIX		*skel_mats ;
  FVECTOR		vec ;
  FMATRIX		*mats = scrpad->mats ;
  EVM_SKEL	*skel ;
  
  skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;
  
  /* オブジェクトにマトリクスを設定する */
  skel = &evmobj->def->skeleton[ first ] ;
  vec.vw = 1.0F ;
  /* モデル情報から親子関係を取得して求める */
  for ( i = first ; i < ( first + n_joints ) ; i++ ){
    FMATRIX	*parent ;
    MT_QuatToMat( mats, skel_rot );
    mats->m[3][0] = skel->rt_tx + skel_trans->vx ;
    mats->m[3][1] = skel->rt_ty + skel_trans->vy ;
    mats->m[3][2] = skel->rt_tz + skel_trans->vz ;
    
    parent = &skel_mats[ skel->parent ] ;
    _sceVu0MulMatrix( mats, parent, mats ) ;
    vec.vx = -skel->rt_tx ;
    vec.vy = -skel->rt_ty ;
    vec.vz = -skel->rt_tz ;
    _sceVu0ApplyMatrix( &mats->m[3][0], mats, &vec );
    skel_mats[i] = *mats ;
    
    skel_rot++ ;
    skel_trans++ ;
    skel++ ;
    mats++ ;
  }
}

static void AddVector(FVECTOR *ret, FVECTOR *a, FVECTOR *b)
{
  ret->vx = a->vx + b->vx;
  ret->vy = a->vy + b->vy;
  ret->vz = a->vz + b->vz;
  ret->vw = (a->vw + b->vw) / 2;
}

/* ベクトルの線形補間 */
static void InterpVector( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t )
{
#if 0 //BP_ASM
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
  res->vw = from->vw;
#endif
}

/* 指定した名前のコントロールを検索 */
static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}
	printf("search faild control !!\n");
	return ( NULL );
}


/* ---------------------------------------------------------------- */
	/*
		口パクに必要なモーションの用意
	*/
static void SetLipMotions(Work *work, int id[7])
{
  /* 各母音に対応するモーションの用意 */
  SetMtnMotionData(&work->lips.mtn_a, id[0]);
  SetMtnMotionData(&work->lips.mtn_i, id[1]);
  SetMtnMotionData(&work->lips.mtn_u, id[2]);
  SetMtnMotionData(&work->lips.mtn_e, id[3]);
  SetMtnMotionData(&work->lips.mtn_o, id[4]);
  SetMtnMotionData(&work->lips.mtn_N, id[5]);
  SetMtnMotionData(&work->lips.mtn_v, id[6]);
}

static void MtnCounterInc(MTN_DATA * motion)
{
  motion->count++;
  if(motion->count > motion->length) motion->count = motion->length;
}

static void MtnCounterLoop(MTN_DATA * motion)
{
  motion->count++;
  if(motion->count > motion->length) motion->count = 0;
}

static void MtnCounterReset(MTN_DATA * motion)
{
  motion->count = 0;
}

/* -------------------------------------------------------------------

           表情モーション関連の関数群

   ------------------------------------------------------------------- */

/*
 * 表情モーションの初期化
 */
static void InitFaceMotion(Work *work)
{
  int i;
  for(i = 0; i < 2; i++)
    {
      work->mtn_face_loaded[i] = 0;    /* 初期状態では表情無し    */
      work->face_rate[i]       = 0.0;  /* 表情のレートを 0 にする */
    }
  work->face_status = 0;
  work->face_morf_rate = 0.0;
}

/*
 * 目パチ処理の初期化
 */
static void InitEyesAnimation(Work * work, int eyes_id)
{
  work->eyes_flags = EYES_FLAG_ENABLE;     /* 自動目パチを許可             */
  work->eyes_t = 0;                        /* 経過時刻を初期化             */
  work->eyes_counter = EYES_BLANK_MIN;     /* 最初の目パチまでの時間を設定 */
  work->eyes_rate = 0.0;                   /* 初期状態のレートは全開にする */
  work->eyes_status = EYES_STAT_OPEN;      /* 初期状態では、目が開いている */
  work->eyes_times  = 0;
  /* 閉じた目のモーションをロード */
  SetMtnMotionData( &work->mtn_eyes, eyes_id);
}

/*
 * 目パチ開始の指示を出す関数
 */
static void StartEyeAnimation(Work * work)
{
  work->eyes_status  = EYES_STAT_CLOSING;  /* 閉じ途中                   */
  work->eyes_t       = 0;                  /* 経過時間をリセット         */
  work->eyes_spd     = 0;
  work->eyes_rate    = 0.0;
}

/*
 * 自動目パチのトリガ処理
 */
static void AutoEyesAnimation(Work * work)
{
  int t;

  work->eyes_counter -= TIME_BASE;
  work->eyes_t       += TIME_BASE;
  
  /* 目パチの途中であれば何もしない */
  if(work->eyes_status != EYES_STAT_OPEN) return;

  /* 最後の目パチから経過した時刻を求める */
  t = work->eyes_t;

  /* 最後の目パチからあまり時間が経過していなければ何もしない */
  if(t < EYES_BLANK_MIN) return;

  /* 目パチの待ち時間として設定された時刻が経過していなければ何もしない */
  if(work->eyes_counter > 0) return;

  /* 次の目パチまでの時間を設定 */
  work->eyes_counter = EYES_BLANK_MIN + EYES_RND(EYES_BLANK_MERGINE);

  if(work->eyes_flags & EYES_FLAG_ENABLE) work->eyes_times++;
}

/*
 * 目パチの動作プロセス
 */
static void EyesAnimationProc(Work * work)
{
  switch(work->eyes_status)
    {
    case EYES_STAT_OPEN:     /* 開いているときは、能動的には何もしない */
      work->eyes_rate = 0.0;

      /* 目パチ開始の指示 */
      if(work->eyes_times > 0) StartEyeAnimation(work);

      break;
    case EYES_STAT_CLOSING:  /* 閉じている途中の場合,
				完全に閉じるまでまぶたを動かす         */
      /* レートの計算 */
      work->eyes_rate += ((float)TIME_BASE / (float)EYES_CLOSE_SPEED);
      if(work->eyes_rate >= 1.0) work->eyes_rate = 1.0;

      /* 時間の処理 */
      if((work->eyes_spd += TIME_BASE) >= EYES_CLOSE_SPEED)
	{
	  work->eyes_status = EYES_STAT_CLOSED;
	  work->eyes_spd -= EYES_CLOSE_SPEED;
	}
      break;
    case EYES_STAT_CLOSED:   /* 閉じている場合は、開くプロセスに移行   */
      /* 確実に閉じているフレームを表示後、開くプロセスに移行 */
      work->eyes_status = EYES_STAT_OPENING;
      break;
    case EYES_STAT_OPENING:  /* 開いている途中の場合、
				完全に開くまでまぶたを動かす           */

      /* レートの計算 */
      work->eyes_rate -= ((float)TIME_BASE / (float)EYES_OPEN_SPEED);
      if(work->eyes_rate <= 0.0) work->eyes_rate = 0.0;

      /* 時間の処理 */
      if((work->eyes_spd += TIME_BASE) >= EYES_OPEN_SPEED)
	{
	  work->eyes_status = EYES_STAT_OPEN;
	  work->eyes_spd = 0;
	  work->eyes_times--;
	  if(work->eyes_times < 0) work->eyes_times = 0;
	}
      break;
    }
#ifdef FACE_DEBUG
  if(work->eyes_status != EYES_STAT_OPEN)
    printf("eyes_rate = %8.3f\n", work->eyes_rate);
#endif /* FACE_DEBUG */
}

static FVECTOR * get_point_trans(MTN_DATA *mtn, int point)
{
  static FVECTOR blank_trans = {0,0,0,1};
  if(mtn == NULL) return &blank_trans;
  if(point >= mtn->joints) return &blank_trans;
  return &mtn->trans[( mtn->length + 1 ) * point + mtn->count ];
}

static FVECTOR * get_point_rot(MTN_DATA *mtn, int point)
{
  static FVECTOR blank_rot = {0, 0, 0, 1};
  if(mtn == NULL) return &blank_rot;
  if(point >= mtn->joints) return &blank_rot;
  return &mtn->rots[( mtn->length + 1 ) * point + mtn->count ];
}

static MTN_DATA * get_motion_by_lip(Work * work, LipID id)
{
  switch(id)
    {
    case LIP_A: return &work->lips.mtn_a;
    case LIP_I: return &work->lips.mtn_i;
    case LIP_U: return &work->lips.mtn_u;
    case LIP_E: return &work->lips.mtn_e;
    case LIP_O: return &work->lips.mtn_o;
    case LIP_N: return &work->lips.mtn_N;
    case LIP_V: return &work->lips.mtn_v;
    case LIP_S: return NULL;             
    case LIP_unknown: return NULL;
    }
  return NULL;
}

static int get_mouth_motion(Work * work, int now_id, float time_rate,
			float *rate, MTN_DATA **mtn_a, MTN_DATA **mtn_b)
{
  struct SLipStruct *mtn_cnv = get_lip_motion();
  BlendInfo *blend;
  int i, j;
  int num = -1;

  blend = NULL;
  for(i = 0; mtn_cnv[i].phone_id != PHONE_unknown; i++)
    if(mtn_cnv[i].phone_id == now_id)
      {
	for(j = 0; mtn_cnv[i].seq[j].time < 1.0; j++)
	  if(mtn_cnv[i].seq[j].time <= time_rate)
	    {
	      blend = &mtn_cnv[i].seq[j].mtn;
	      num = j;
	    }
	*rate = blend->rate;
	*mtn_a = get_motion_by_lip(work, blend->motion_a);
	*mtn_b = get_motion_by_lip(work, blend->motion_b);
	return num;
      }
  return -1;
}

static int get_mouth_motion_by_step(Work * work,int id, int step, 
				    float *rate,
				    MTN_DATA **mtn_a, MTN_DATA **mtn_b)
{
  struct SLipStruct *mtn_cnv = get_lip_motion();
  BlendInfo *blend;
  int i, j;
  int num = -1;

  for(i = 0; mtn_cnv[i].phone_id != PHONE_unknown; i++)
    if(mtn_cnv[i].phone_id == id)
      {
	j = step;
	if(step < 0)
	  {
	    j = 0;
	    while(mtn_cnv[i].seq[j + 1].time < 1.0) j++;
	  }
	blend = &mtn_cnv[i].seq[j].mtn;
	num = j;
	*rate = blend->rate;
	*mtn_a = get_motion_by_lip(work, blend->motion_a);
	*mtn_b = get_motion_by_lip(work, blend->motion_b);
	return num;
      }
  return -1;
}
static float calc_time_rate(int id, int step, float time_scale)
{
  struct SLipStruct *mtn_cnv = get_lip_motion();
  int i;
  float begin_time, nxt_time;
  float rate;
  begin_time = 0.0;
  nxt_time = 1.0;
  for(i = 0; mtn_cnv[i].phone_id != PHONE_unknown; i++)
    if(mtn_cnv[i].phone_id == id)
      {
	begin_time = mtn_cnv[i].seq[step].time;
	nxt_time = mtn_cnv[i].seq[step + 1].time;
	break;
      }

  rate = (float)(time_scale - begin_time) / (float)(nxt_time - begin_time);
  if(rate > 1.0) rate = 1.0;

  return rate;
}

/*
 * 表情のモーフィング設定を行う
 *
 * face_id     変更後の表情
 * rate        変更後の表情の強さ
 *             ※ 変更後表情と、ニュートラル状態の補間レート。
 *                0.0 に近い程ニュートラル状態に近くなる。
 * time        表情の変更に要する時間(1/300単位)
 */
static void FaceMorfingSetup(Work * work, int face_id, float rate, int time)
{
  int face;

  /* メインの表情を切替え、モーフィング中のフラグをセットする */
  work->face_status = (work->face_status ^ 1) | FACE_STATUS_MORFING;

  work->face_morf_time = time;   /* モーフィングの所要時間 */
  work->face_time_cnt  = 0;      /* 経過時間をリセット     */
  face = work->face_status & 1;
  
  /* 新しいモーションの読み込み */
  if(face_id >= 0)
    {
      SetMtnMotionData(&work->mtn_face[face], face_id);
      MtnCounterReset(&work->mtn_face[face]);
      work->mtn_face_loaded[face] = 1;
    }
  else
    work->mtn_face_loaded[face] = 0;
  
  /* 表情自体のレート設定 */
  work->face_rate[face] = rate;

  /* モーフィング開始直後なので、レートは 0.0 */
  work->face_morf_rate = 0.0;
}

/*
 * 各フレームにおける、表情モーフィング処理
 */
static void FaceMorfingProc(Work * work)
{
  int cnt;

  /* モーフィング中でなければ何もしない */
  if(!(work->face_status & FACE_STATUS_MORFING)) return;

  /* 以下の処理は、将来的に tick に置き換え */
  /* ここから */
  work->face_time_cnt += TIME_BASE;
  cnt = work->face_time_cnt;
  /* ここまで */


  work->face_morf_rate = (float)cnt / (float)work->face_morf_time;

  if(work->face_morf_rate >= 1.0)
    {
      work->face_morf_rate = 0.0;
      work->face_status &= 1;
    }
}


static void get_face_motion(Work * work, FVECTOR **trans, FVECTOR **rot,
			    int i, int face,
			    FVECTOR *blank_trans, FVECTOR *blank_rot)
{
  if(!work->mtn_face_loaded[face])
    {
      *trans = blank_trans;
      *rot = blank_rot;
    }
  else
    {
      *trans = get_point_trans(&work->mtn_face[face], i);
      *rot   = get_point_rot(&work->mtn_face[face], i);
    }
}



/* 
 * 表情と口パク,目を統合する
 * 以前より改良し、Work には補間 rate のみを保持し、
 * モーション計算を一括して行う。
 */
static void BlendFaceElements(Work * work)
{
  int       i, step;
  int       face;
  int       pre_id, now_id;        /* 直前および今回の口形状が属する音素 ID */
  float     now_time_rate;         /* 今回の口形状用のタイムスケール     */
  float     time_rate;             /* 時間経過に伴う口形状の補間レート   */
  float     mouth_rate, pre_rate;  /* 口形状生成用のレート */

  MTN_DATA *mtn_a, *mtn_b;         /* 口形状生成用のモーションのポインタ */
  MTN_DATA *pre_a, *pre_b;         /* 直前の口形状生成用モーション       */

  FVECTOR blank_rot, blank_trans;  /* ニュートラル状態用 */
  FVECTOR face_rot,  face_trans;   /* 表情計算用         */
  FVECTOR eyes_rot,  eyes_trans;   /* 目の開閉計算用     */

  FVECTOR mouth_rot, mouth_trans;  /* 口計算用           */
  FVECTOR pre_rot,   pre_trans;    /* 直前の口形状用     */

  FVECTOR *n_rot, *n_trans;
  FVECTOR *p_rot, *p_trans;

  face = work->face_status & 1;

  /* blank_trans, blank_rot に、ニュートラル状態の平行移動量、回転量を設定 */
  blank_trans.vx = blank_trans.vy = blank_trans.vz = 0;
  blank_rot.vx = blank_rot.vy = blank_rot.vz = 0;
  blank_trans.vw = blank_rot.vw = 1;

  /*
   * 口形状モーション関係のパラメタを取得
   */
  now_id = work->lip_info->now_phone_id;
  if(work->lip_info->length != 0)
    {
      now_time_rate = (float)work->lip_info->delta_time / (float)work->lip_info->length;
    }
  else now_time_rate = 1.0;

  DBG("<2>\n");

  /* 現在の口形状の要素を得る */
  step = get_mouth_motion(work, now_id,
			  now_time_rate, &mouth_rate, &mtn_a, &mtn_b);
  DBG("<2.5>\n");
  /* 直前の形状から現在の形状までのうち、時間的な補間レートを求める */
  time_rate = calc_time_rate(now_id, step, now_time_rate);
  DBG("<3>\n");

  if(step > 0)
    {
      DBG("<3.5a>\n");
      /* 音素内の形状変遷ステップが 0 では無い場合、
	 同じ音素における直前のステップを最終形状とする */
      pre_id = now_id;
      step--;
    }
  else
    {
      DBG("<3.5b>\n");
      /* 音素の最初の形状であれば、直前の音素の最終ステップを最終形状とする */
      pre_id = work->lip_info->pre_phone_id;
      step = -1;
    }
  if(pre_id == PHONE_unknown) pre_a = pre_b = NULL;
  else
    get_mouth_motion_by_step(work, pre_id, step, &pre_rate, &pre_a, &pre_b);
  DBG("<4>\n");

  /*
   * 関節一つごとに計算していく。
   */
  for(i = 0; i < 35; i++)
    {
      /*
       * 口モーションの計算
       */
      /* 直前の口形状を作成 */
      InterpVector(&pre_trans,
		   get_point_trans(pre_a, i),
		   get_point_trans(pre_b, i), pre_rate);

      MT_QuatSlerp(&pre_rot,
		   get_point_rot(pre_a, i),
		   get_point_rot(pre_b, i), pre_rate);

      MT_QuatNormalize(&pre_rot, &pre_rot);


      /* 今回の口形状を作成 */
      InterpVector(&mouth_trans,
		   get_point_trans(mtn_a, i),
		   get_point_trans(mtn_b, i), mouth_rate);
      
      MT_QuatSlerp(&mouth_rot,
		   get_point_rot(mtn_a, i),
		   get_point_rot(mtn_b, i), mouth_rate);
      MT_QuatNormalize(&mouth_rot, &mouth_rot);

      /* 時間に従い、直前の口形状から今回の口形状への補間を行う */
      InterpVector(&mouth_trans, &pre_trans, &mouth_trans, time_rate);
      MT_QuatSlerp(&mouth_rot,   &pre_rot,   &mouth_rot,   time_rate);
      MT_QuatNormalize(&mouth_rot, &mouth_rot);

      /* パワーに従い、ニュートラル状態との補間を行う */
      InterpVector(&mouth_trans,
		   &blank_trans, &mouth_trans,
		   work->total_power);

      MT_QuatSlerp(&mouth_rot,
		   &blank_rot, &mouth_rot,
		   work->total_power);

      MT_QuatNormalize(&mouth_rot, &mouth_rot);
      

      /*
       * 目の開閉モーションの計算
       */
      /* 平行移動量 */
      InterpVector(&eyes_trans, &blank_trans,
		   get_point_trans(&work->mtn_eyes, i), work->eyes_rate);

      /* 回転量 */
      MT_QuatSlerp(&eyes_rot, &blank_rot,
		   get_point_rot(&work->mtn_eyes, i), work->eyes_rate);

      MT_QuatNormalize(&eyes_rot, &eyes_rot);

      /*
       * ベクトル加算
       */
      /* 並行移動量補間 */
      AddVector(&work->face_skel_trans[i],
		&mouth_trans,    /* 口の平行移動量 */
		&eyes_trans);    /* 目の平行移動量 */


      /* 回転量補間 */
      AddVector(&work->face_skel_rot[i],
		&mouth_rot,    /* 口の回転角     */
		&eyes_rot);    /* 目の回転角       */

      /* 
       * 表情モーションの計算
       */
      get_face_motion(work, &n_trans, &n_rot,
		      i, face, &blank_trans, &blank_rot);
      /*
       * 現在の表情を作成
       */
      /* 平行移動量 */
      InterpVector(&face_trans, &blank_trans, n_trans, work->face_rate[face]);
      
      /* 回転量 */
      MT_QuatSlerp(&face_rot, &blank_rot, n_rot, work->face_rate[face]);
      MT_QuatNormalize(&face_rot, &face_rot);
      
      if(work->face_status & FACE_STATUS_MORFING)
	{
	  /*
	   * 直前の表情を作成
	   */
	  get_face_motion(work, &p_trans, &p_rot,
			  i, face ^ 1, &blank_trans, &blank_rot);
	  InterpVector(&pre_trans,
		       &blank_trans, p_trans, work->face_rate[face ^1]);
	  MT_QuatSlerp(&pre_rot,
		       &blank_rot, p_rot, work->face_rate[face ^1]);
	  MT_QuatNormalize(&pre_rot, &pre_rot);


	  /*
	   * 現在の表情と補間をとる
	   */
	  InterpVector(&face_trans,
		       &pre_trans, &face_trans, work->face_morf_rate);
	  MT_QuatSlerp(&face_rot,
		       &pre_rot, &face_rot, work->face_morf_rate);
	  MT_QuatNormalize(&face_rot, &face_rot);
	}
      /*
       * 表情を目と口の加算結果に対し、さらに加算
       */
      AddVector(&work->face_skel_trans[i],
		&work->face_skel_trans[i], /* 口と目の平行移動量加算結果 */
		&face_trans);              /* 表情の平行移動量           */
      
      AddVector(&work->face_skel_rot[i],
		&work->face_skel_rot[i],    /* 目と表情の回転角加算結果 */
		&face_rot);                 /* 表情の回転角             */

      MT_QuatNormalize(&work->face_skel_rot[i], &work->face_skel_rot[i]);
    }
}




/* ---------------------------------------------------------------- */
	/*
		キャラアクター関数
	*/
static void Act( Work *work )

{
  int		n_msg;
  GV_MSG	*msg ;
  DG_EVMOBJ	*evmobj ;

  DBG("entering Act() in face.c\n");

  /* メッセージチェック */
  if( n_msg = GV_ReceiveMessage( work->name, &msg ) )
    {
      for ( ; n_msg > 0 ; n_msg--, msg++ ){
	switch ( msg->message[0] )
	  {
	  case 0:/* 表情の設定と、直前の表情と補間しながらの変更 */
	    {
	      int face_id;
	      int time;
	      float rate;

	      face_id = msg->message[1];                   /* 表情モーション */
	      rate = (float)msg->message[2] / (float)1000; /* 表情の強さ     */
	      time = msg->message[3];                      /* 補間時間       */
	      if(rate < 0) rate = 0.0;
	      if(rate > 1.0) rate = 1.0;
	      FaceMorfingSetup(work, face_id, rate, time);
	    }
	    break ;
	  case 1: /* 表情をニュートラルに変更 */
	    {
	      int time;
	      time = msg->message[1];                      /* 補間時間       */
	      FaceMorfingSetup(work, -1, 1.0, time);
	    }
	    break;
	  case 2: /* 口パク強度設定 */
	    {
	      work->total_power = (float)msg->message[1] / (float)1000;
	      if(work->total_power < 0) work->total_power = 0;
	      if(work->total_power > 1.0) work->total_power = 1.0;
	    }
	    break;
	  case 3: /* 強制目パチ */
	    {
	      int times;
	      times = msg->message[1];                     /* 目パチ回数 */
	      work->eyes_times = times;
	    }
	    break;
	  case 4: /* 自動目パチの禁止 */
	    {
	      work->eyes_flags &= 0xffffffff ^ EYES_FLAG_ENABLE;
	    }
	    break;
	  case 5: /* 自動目パチの許可 */
	    {
	      work->eyes_flags |= EYES_FLAG_ENABLE;
	    }
	    break;
	  }
      }
    }
  
  evmobj = work->evmobj ;

  /* 時系列による、口パクモーションの自動生成 */
  work->lip_info = GetLipInfo();  /* 口パク情報を得る */

  /* 自動目パチのプロセス */
  AutoEyesAnimation(work);
  EyesAnimationProc(work);

  /* 表情のモーフィングプロセス */
  FaceMorfingProc(work);

  /* 表情モーションと、口パクモーション,目パチモーションを加算したものが、
     そのフレームの最終的な顔モーションとなる。              */
  BlendFaceElements(work);

  /* 顔モーション設定 */
  //EvmActMotionImmediate( work->evmobj, 21, work->mtn_data.joints, work->face_skel_trans, work->face_skel_rot );
  EvmActMotionImmediate( work->evmobj,
			 21,  /* 顔のマトリクスは２１番目から（固定） */
			 32,  /* 顔用マトリクス数は３２個（固定）     */
			 work->face_skel_trans,	/* マトリクスの平行移動成分（×３２個） */

			 work->face_skel_rot );	/* マトリクスの回転成分（×３２個） */
  
}

/* ---------------------------------------------------------------- */
	/*
		キャラアクター終了関数
	*/
static void Die( Work *work )
{
}

/* ---------------------------------------------------------------- */
	/*
		キャラクタ初期化関数
	*/
static int GetResources_S( Work *work, int name, int where )
{
	int		chara_id ;
	CONTROL	*ctrl ;
	OBJECT	*object ;

	work->name = name ;

	/* キャラクタ取得 */
	if ( GCL_GetOption( 'c' ) == NULL ) return -1;
	chara_id = GCL_GetNextInt() ;
	ctrl = SearchControl( chara_id );
	if ( ctrl == NULL ){
		printf("e3_fanim.c: search error control\n");
		return ( -1 );
	}
	if ( ctrl->object == NULL ){
		printf("e3_fanim.c: control initialize miss error\n");
		return ( -1 );
	}
	object = ctrl->object ;
	if ( object->evmobj == NULL ){
		printf("e3_fanim.c: error: no support chara\n");
		return ( -1 );
	}

	work->evmobj = object->evmobj ;

	work->eye_pos_type = -1 ;
	//work->eye_pos_type = 2 ;

	return ( 0 );
}

static int GetResources_P( Work *work, int name, DG_EVMOBJ *evmobj )
{
	work->name = name ;
	work->evmobj = evmobj ;

	work->eye_pos_type = -1 ;

	return 0;
}

/* ---------------------------------------------------------------- */
	/*
		シナリオインターフェイス
	*/
void *NewFaceMotionSet( int name, int where )
{
  Work * work ;
  int    motions[8];
  int    eyes;
  int    i;
  
  /* 口パク用モーションのうけとり */
  if(GCL_GetOption('a') != NULL)
    {
       // We need to copy into the motions array, but
       // the source is endian swapped
       int *motions_le = GCL_GetNextResource_LE();

       for ( i = 0; i < sizeof(motions)/sizeof(motions[0]); ++i )
       {
          BP_LE_Swap4Bytes( motions + i, motions_le + i );
       }

       /* 目パチ用モーションの受取 */
       eyes = BP_LE_SwapSInt( motions[7] );
    }
  else
    return NULL;
  
  OPERATOR() ;
  work = (Work *)GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 0xf1 ) ;
  if ( work != NULL ) {
    
    /* 目の初期化 */
    InitEyesAnimation(work, eyes); /* 目パチのロードと初期化     */
    
    /* 口の初期化 */
    SetLipMotions(work, motions);  /* 各母音の口の形をロードする */
    work->total_power = 1.0;  /* 口の動きは最大に */
    
    /* 表情の初期化 */
    InitFaceMotion(work);
    
    GV_SetActor( &( work->actor ), Act, Die ) ;
    GV_ActorEX( &work->actor );
    if ( GetResources_S( work, name, where ) < 0 ) {
      GV_DestroyActor( work ) ;
      return NULL ;
    }
  }
  
  return (void *)work ;
}

/* ---------------------------------------------------------------- */
	/*
		プログラムインターフェイス
	*/

void *NewFaceMotion( int name, DG_EVMOBJ *evmobj,
		     int mouth[7], int eyes )
{
  Work		*work ;
  
  OPERATOR() ;
  
  work = (Work *)GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( Work ), 0xf1 ) ;
  if ( work != NULL ) {
    /* 目の初期化 */
    InitEyesAnimation(work, eyes); /* 目パチのロードと初期化     */
    
    /* 口の初期化 */
    SetLipMotions(work, mouth);  /* 各母音の口の形をロードする */
    work->total_power = 1.0;  /* 口の動きは最大に */
    
    /* 表情の初期化 */
    InitFaceMotion(work);
    
    GV_SetActor( &( work->actor ), Act, Die ) ;
    GV_ActorEX( &work->actor );
    if ( GetResources_P( work, name, evmobj ) < 0 ) {
      GV_DestroyActor( work ) ;
      return NULL ;
    }
  }
  return (void *)work ;
}

void GM_SetLipPower(Work * work, float power)
{
  work->total_power = power;
}
