//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  face_bug.c
  バグり顔管理キャラ

  2001/07/18   Y.Kira
  $Id: face_bug.c,v 1.1.1.3 2002/11/19 11:45:02 Yoshizawa1 Exp $
*/
/*
  ランダムな間隔でノイズとともにモデルを入れ換えます。
  指定された話者名によっては、ちょっとしたイタズラが入っています。
*/

#ifdef PSX2
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
#endif

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
#define _face_bug_c_
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

#include "face_bug.h"
#include "codec_config.h"
#include "cdc_hair.h"
#include "../../kano/hair/hair_called.h"
#include "cdc_noise.h"

#ifdef PSX2
#undef _DEBUG_ 
#ifdef _DEBUG_
#define DBG(...)  printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* _DEBUG_ */
#endif
#ifdef KP_XBOX
#define DBG
#endif


#define TIME_MAX 250

#define BUG_RATE 10

typedef struct _BugHairInfo {
  struct _BugHairInfo * next;
  void * model;
  int    sample_num;
  void * bound;
  int    hit_param;
  int    ambient;
  int    nowind;
} BugHairInfo;


typedef struct {
  GV_ACT_EX    actor;

  int          side;       /* 表示サイド                 */
  int          speaker;    /* 話者名                     */
  int          dummy_bone; /* 骨格名                     */

  int          time_min;   /* 切替えの最低間隔           */
  int          bias_time;  /* 最大上乗せ時間             */

  int          next_time;  /* 次の顔に切替えるまでの時間 */
  int          next_cnt;   /* 顔切替えの時間カウンタ     */

  int          defmtn_id;  /* デフォルトモーション ID    */
  int          now_face;   /* 現在指定されている顔       */
  int          face_max;   /* 登録されている顔の数       */
  int        * face_list;  /* 顔名称リスト               */


  int          tri_handle; /* 特別な人のテクスチャ(謎)   */
  SPR_OBJ    * special_spr;/* 特別な人の特別な演出       */

  void       * face_work;  /* 顔のワーク                 */

  BugHairInfo * bug_hair;  /* バグ顔についている髪の毛情報 */

  int          special:1;  /* 特別な顔が用意されている人の証 */
  int          finish:1;   /* 終了フラグ                     */
} Work;

/* テクスチャ名をあえて即値でハードコーディング */
static int face_2d_tex_list[] = {
  0x005c68db,
  //  0x003914b1,
  0x008fc20d
};

static struct {
  int zfar;
  int zoom;
  int heading;
  int pan;
  int pitch;
  int gain;
} face_2d_camera_list[] = {
  /* あえてどれがなんだか書かない */
  { 1000000, 10565,  -18,  -1477,  -2,   9897 },
  // { 1000000, 12890,  176, -36977,   5,   -503 },
  //{ 1000000, 11665, -303,  21422,   5, -21203 }  // 旧 GB大佐設定
  { 601793, 7250, -262, 14421, 6, 10796 }
};

static Work * now_work[2] = {NULL, NULL};



void codecSetupBugHair(int side,
		       void * model, int sample_num,
		       void * bound, int hit_param, int ambient, int nowind)
{
  BugHairInfo * info;
  Work * work = now_work[side];

  if(NULL == work) return;
  if(NULL == (info = codecMalloc(sizeof(BugHairInfo)))) return;

  info->model      = model;
  info->sample_num = sample_num;
  info->bound      = bound;
  info->hit_param  = hit_param;
  info->ambient    = ambient;
  info->nowind     = nowind;

  info->next = work->bug_hair;
  work->bug_hair = info;

  /* 現在の顔に付加 */
  codecSetupHair(side, model, sample_num, bound, hit_param, ambient, nowind);
}


static void set_2d_camera(Work * work, int num)
{
  float zfar, zoom;
  float heading, pan;
  float gain, pitch;

  zfar  = (float)face_2d_camera_list[num].zfar  / 1000.0F;
  zoom = (float)face_2d_camera_list[num].zoom / 1000.0F;

  heading = (float)face_2d_camera_list[num].heading;
  pan     = (float)face_2d_camera_list[num].pan / 1000.0F;

  pitch = (float)face_2d_camera_list[num].pitch;
  gain  = (float)face_2d_camera_list[num].gain / 1000.0F;

  heading = (heading * 2.0F * (float)M_PI) / 4096.0F;
  pitch   = (pitch * 2.0F * (float)M_PI) / 4096.0F;

  cdcFaceCamera(work->side, zfar, zoom, heading, pan, pitch, gain);
}

static void Act(Work * work)
{
  /* 終了が指定されていればなにもせずに終了 */
  if(work->finish)
    {
      GV_DestroyActor(work);
      return;
    }

  work->next_cnt += TIME_BASE;
  if(work->next_cnt >= work->next_time)
    {
      /* 時間が来たら、他の顔に入れ換える */
      int max = work->face_max;
      int face;

      work->next_cnt -= work->next_time;

      if(work->special)
	if(((BP_PS2_rand() >> 16) % 100) < BUG_RATE)
	  {
	    face = (BP_PS2_rand() >> 16) % 2; // 3;
	    NewCodecNoise(work->side, CDC_NOISE_VFAST, 3);
	  
	    /* この状況が発生するのは「あの人」の場合だけ */
	    SPR_ObjSetTexture(work->special_spr,
   			      face_2d_tex_list[face],
			      work->tri_handle);
	    SPR_SetColorSprite(work->special_spr, 128, 128, 128, 128);
	    SPR_SHOW(work->special_spr);

	    /* バックで顔と同じカメラアングルに切替える */
	    set_2d_camera(work, face);
	    
	    work->now_face = -1;
	    work->next_time = ((BP_PS2_rand() >> 16) % 10) + 25;
	    return;
	  }

      face = (BP_PS2_rand() >> 16) % (max - 1);
      if(face >= work->now_face) face++;
      work->now_face = face;

      if(work->special) SPR_HIDE(work->special_spr);
      {
	int noise_type;

	noise_type = ((BP_PS2_rand() >> 16) % 3) ? CDC_NOISE_VFAST : CDC_NOISE_SAND;
	NewCodecNoise(work->side, noise_type, 3);
      }
      
      work->face_work = NewCodecFace(0,
				     work->dummy_bone,
				     work->side,
				     work->face_list[face],
				     work->speaker);

      /* 髪の毛パーツが指定されている場合は、髪の毛を付け直す。 */
      {
	BugHairInfo * hair;

	/* 付加されている髪の毛パーツを,全て付け直す。 */
	for(hair = work->bug_hair; hair != NULL; hair = hair->next)
	  codecSetupHair(work->side,
			 hair->model, hair->sample_num,
			 hair->bound, hair->hit_param,
			 hair->ambient, hair->nowind);
      }

      DBG("bug face speaker = 0x%08x(%d)\n", work->speaker, work->speaker);
      if(work->defmtn_id >= 0)
	codecSetDefaultMotion(work->side, work->defmtn_id);

      /* 最低でも 0.5～2秒 */
      work->next_time =
	((BP_PS2_rand() >> 16) % work->bias_time) + work->time_min;

      /* 正常な顔だけ長く */
      if(!face) work->next_time = work->next_time * 3 / 2;
    }
}

static void Die(Work * work)
{
  if(work->special)
    {
      SPR_Destroy_2D_Object(work->special_spr);
      SPR_KillTexture(work->tri_handle);
    }

  /* 髪の毛パーツが付加されていたら、全て開放する*/
  {
    BugHairInfo * hair;
    BugHairInfo * next;

    hair = work->bug_hair;
    while(NULL != hair)
      {
	next = hair->next;
	codecFree(hair);
	hair = next;
      }
  }
  if(work == now_work[work->side]) now_work[work->side] = NULL;
}


static int ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;
  switch(signal)
    {
    case SIGNAL_BUGFACE_KILL:
      work->finish = 1;
      break;
    case SIGNAL_BUGFACE_DEFMTN:
      if((value >> 24) == work->side) work->defmtn_id = value & 0xffffff;
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

void * BugFaceGetFaceWork(void * workp)
{
  Work * work = workp;

  return work->face_work;
}

static int GetResources(Work * work,
			int side, int dummy_bone,
			int speaker, int min, int bias,
			int * model_list)
{
  int cnt;
  int i;

  /* 終了を指定するためのシグナルハンドラを設定する */
  GV_SetActorSignalFunc(work, ReceiveSignal);

  work->side = side;
  work->bug_hair = NULL;   /* 最初は髪の毛は付いていない */
  work->special = 0;
  work->finish = 0;
  work->speaker = speaker;

  work->time_min = 150;
  if(min >= 0) work->time_min = min;

  work->bias_time = 250;
  if(bias >= 0) work->bias_time = bias;

  /*
   * コードを読んだだけでは誰だかわからないように、あえて即値で書く。
   */
  if(speaker == 0x0064b3f4)   /* さぁ、誰でしょう(笑) */
    {
      work->special = 1;   /* 特別ゲスト */
      work->tri_handle = SPR_LoadTexture(0x7232c8);
      work->special_spr = SPR_Create_2D_Object(SP_SPRITE, 2 + side, NULL);
      ASSERT(NULL != work->special_spr);
#if 0 //BP_PS2
//#ifdef PSX2
      SPR_SetPosSprite(work->special_spr, &(SPR_POS){0.0F, 0.0F});
#endif
#ifdef KP_XBOX
	  {
		  SPR_POS p = {0.0F, 0.0F};
	      SPR_SetPosSprite(work->special_spr, &p);
	  }
#endif
      SPR_SetSizeSprite(work->special_spr, 512.0F, 384.0F);
      SPR_SetColorSprite(work->special_spr, 128, 128, 128, 128);
      SPR_SET_FLAGS(work->special_spr, SPR_FLAG_PRIV);
      SPR_HIDE(work->special_spr);  /* 通常は隠しておく */
    }

  /* モデルリスト中のモデル数を数える */
  cnt = 0;
  for(i = 0; model_list[i] >= 0; i++) cnt++;
  work->face_max = cnt;

  /* 次までの時間 */
  work->next_time = (BP_PS2_rand() >> 16) % work->bias_time + work->time_min;
  work->next_time = work->next_time * 3 / 2;
  work->next_cnt  = 0;

  /* 現在の顔 */
  work->dummy_bone = dummy_bone;
  work->face_list = model_list;
  work->now_face = 0;  /* 最初の顔がデフォルト */
  work->face_work = NewCodecFace(0, dummy_bone, side, model_list[0], speaker);
  work->defmtn_id = -1;
  DBG("bug face speaker = 0x%08x(%d)\n", work->speaker, work->speaker);
  now_work[side] = work;
  return 0;
}

void * NewBugFace(int side, int dummy_bone, int speaker,
		  int min, int bias, int * model_list)
{
  Work * work = codecActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf0);
  if(NULL == work) return NULL;
  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, side, dummy_bone, speaker,
		  min, bias, model_list))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}
