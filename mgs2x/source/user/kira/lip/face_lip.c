//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
        リップアニメーションドライバ

	2000/07/17 Yoshihito Kira
	$id:$
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <libgraph.h>
#endif

#include "libgv.h"
#include "libgv.cnf"
#include "libfs.h"
#include "stream.h"
#include "g_define.h"

#include "sd_ee.h"
#include "g_sound.h"

#include "strctrl.h"

#include "face_lip.h"


#ifdef _DEBUG_
#define DBG(args...)  printf(args)
#else
#define DBG(...)
#endif /* _DEBUG_ */




/*
 * ストリームデータとして記録されている、時系列の情報
 */
typedef struct {
  unsigned short time;      /* パケット先頭からの時系列 */
  unsigned char  phone_id;  /* 音素 ID                  */
  unsigned char  power;     /* パワー                   */
} PhoneSegment;


/* 絶対時間取得マクロ */
#define TICK_TIME(work)   ((work)->ctrl->tick)

/* パケットにおける最大のシーケンスステップ数 */
#define MAX_LIPSTEP  ((LIP_PACKET_SIZE - 16) / sizeof(PhoneSegment))

/*
  最小の音素長。記録されている音素長がこの長さより
  短い場合は補正を行う。
*/
#define MIN_STEP  5

/*
 * 無音部の口閉じ速度。
 * 無音部の長さがこの長さ以上であれば、
 * 口モーション生成情報はこの長さを元にして生成する。
 */
#define CLOSE_SPEED 10


typedef struct PacketBuf {
  u_long64 size;
  u_long64 base_time;
  u_long64 flags;
  PhoneSegment lip_seq[ MAX_LIPSTEP ];
} PacketBuf;

typedef struct {
  GV_ACT_EX  actor;
  int    type;                   /* ストリームのタイプ */
  void * stream_h;               /* */
  int    handle;

  GM_STREAM_CONTROL * ctrl;
  int           base_time;       /* 現在処理中のパケットの先頭絶対時刻 */
  u_long64 flags;           /* 現在処理中のパケットのフラグ       */

  PhoneSegment * lip_seq;        /* 口パク時系列データ */
  PhoneSegment * pos;            /* 現在のポジション   */


  int          final_phone_id;   /* 最後のパケットにおける最後の phone_id */
  int          final_time;       /* 最後のパケットにおける最後の time     */


  /* 処理中の音素の情報 */
  int          last_basetime;    /* 開始時刻           */
  int          last_length;      /* 音素の長さ         */
  /*
    時刻が (last_basetime + last_length) 以上にならない限り、
    次の音素には移行しない。
  */

  int          pre_phone_id;     /* 直前に再生していた音素 ID */
  int          now_phone_id;     /* 現在再生中の音素 ID       */

  float        pre_power;        /* 直前の音素のパワー        */
  float        now_power;        /* 現在のパワー              */
  LipInfo      info;

} Work;

static int using_handle = 0;  /* 初期状態では 0 */
static Work * works = NULL;  /* テスト用の work */

#define PACKET_BUF_MASK 0x07
#define PACKET_BUF_MAX  (PACKET_BUF_MASK + 1)

static PacketBuf packet_buf[ PACKET_BUF_MAX ];
static int       packet_read_idx;
static int       packet_get_idx;
static int       packet_no_more;  /* これ以上パケットが無い場合に非0 になる */

static void EndianSwapPhoneSegment( PhoneSegment *pData )
{
   BP_LE_SwapUShort_Inp( &pData->time );
}

/*
 * パケットデータ本体を、ローカル領域にコピーする
 */
static void takeout_lip_sequence(Work * work, void * data)
{
  PhoneSegment * seq;
  int i;

  packet_buf[packet_read_idx].base_time = FS_STREAM_GET_TIME(data);
  packet_buf[packet_read_idx].size      = FS_STREAM_GET_SIZE(data);
  packet_buf[packet_read_idx].flags     = FS_STREAM_GET_OPTION(data);

  /* 最終パケットであれば、これ以上は読み込まないようにする */
  if(packet_buf[packet_read_idx].flags & FLAGS_FINAL_PACKET)
    {
      printf("<LIP>: read no more.\n");
      packet_no_more = 1;
    }

  seq = data;
  i = 0;
  do {
    memcpy(&packet_buf[packet_read_idx].lip_seq[i],
	   &seq[i], sizeof(PhoneSegment));

    EndianSwapPhoneSegment( &seq[i] );
  } while(seq[i++].time != TIME_term);
  
  packet_read_idx++;
  packet_read_idx &= PACKET_BUF_MASK;
  FS_StreamFreeData(work->stream_h, data);
}

/*
 * 新しいパケットをロードする
 */
static void load_new_packet(Work * work)
{
  void *ptr;

  if(packet_read_idx != packet_get_idx && !packet_no_more)
    {
      if(NULL != (ptr = FS_StreamGetData(work->stream_h, work->type)))
	takeout_lip_sequence(work, ptr);
    }
}

/* ポインタを次のパケットに進行させる */
static int next_packet(Work * work)
{
  if(packet_get_idx + 1 == packet_read_idx) return -1;
  packet_get_idx ++;
  packet_get_idx &= PACKET_BUF_MASK;
  work->lip_seq   = packet_buf[packet_get_idx].lip_seq;
  work->base_time = packet_buf[packet_get_idx].base_time;
  work->flags     = packet_buf[packet_get_idx].flags;
  return 0;
}

/*
 * 現在の時刻におけるセグメント情報を得る
 */
static void get_lip_info(Work * work, LipInfo * info)
{
  int time;

  time = TICK_TIME(work);          /* 音声先頭からの絶対時刻の取得 */
  if(time < 0) time = 0;
  if(time < (work->last_basetime + work->last_length)) goto set_data;

  if(work->pos->time == TIME_term)
    {
      GV_DestroyActor(work);
      goto set_data;
    }

  /*
   * 音素境界を越えた場合の処理
   */
  /* 直前に処理していた音素 ID を更新 */
  work->pre_phone_id  = work->now_phone_id;

  /* 直前に処理していた音素のパワーを更新 */
  work->pre_power     = work->now_power;
  work->last_basetime = work->pos->time * 3 + work->base_time;

  /* length の取得: 記録されているセグメント長が MIN_STEP 未満である場合は、
                    後続のセグメントを追加し、セグメント長を MIN_STEP 以上に
		    する。 */
  work->last_length = 0;
  do {
    /* 最後の音素の形状に合わせる */
    work->now_phone_id  = work->pos->phone_id;
    work->now_power     = (float)work->pos->power / (float)255;
    work->pos++;

    /* パケットの終端にたどりついた場合 */
    if((work->pos)->time == TIME_term)
      {
	if(work->flags & FLAGS_FINAL_PACKET) 
	  {
	    work->last_length = CLOSE_SPEED * 3;
	    work->now_phone_id = PHONE_sil;
	    break;
	  }
	if(next_packet(work))   /* これ以上パケットが無い場合 */
	  {
	    work->last_length = CLOSE_SPEED * 3;
	    work->now_phone_id = PHONE_sil;
	    break;
	  }
	work->pos = work->lip_seq;   /* 新たなセグメント始端に設定 */
      }
    /* 長さを追加 */
    work->last_length += ((work->pos)->time * 3 + work->base_time)
      - work->last_basetime;
  } while(work->last_length < (MIN_STEP * 3));

 set_data:
  info->pre_phone_id = work->pre_phone_id;
  info->now_phone_id = work->now_phone_id;
  info->length       = work->last_length;
  info->begin_time   = work->last_basetime;
  info->delta_time   = time - work->last_basetime;
  info->pre_power    = work->pre_power;
  info->now_power    = work->now_power;


  /*
   * 無音部の補正
   */
  if((info->length >= (CLOSE_SPEED * 3)) &&
     (info->now_phone_id == PHONE_sil))
    {
      if(info->delta_time <= (CLOSE_SPEED * 3))
	info->length = CLOSE_SPEED * 3;
      else
	info->delta_time = info->length;
    }
}


/* ------------------------------------------------------------------------- */


/*
 * ドライバ開放
 */
static void Die(Work * work)
{
  works = NULL;
}

/*
 * メインループ関数
 */
static void Act(Work * work)
{
  /* パケットが読めるのであれば読む */
  load_new_packet(work);

  /* そのフレームにおける口パクの情報を得る */
  get_lip_info(work, &work->info);
  
}

/*
 * 初期化関数
 */
static int LipStreamInit(Work * work)
{
  packet_read_idx = 0;
  packet_get_idx  = -1;
  packet_no_more  = 0;
  /*
   * 最初のパケットを得る
   */
  load_new_packet(work);
  next_packet(work);
  work->pos = work->lip_seq;
  work->pre_phone_id = work->now_phone_id = PHONE_sil;
  work->last_basetime = 0;
  work->last_length = work->pos->time * 3 + work->base_time;
  return 0;
}

void * NewStreamLipDriver(GM_STREAM_CONTROL *ctrl, int type)
{
  Work * work;

  if(NULL != (work = GV_NewActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf2)))
    {
      GV_SetActor( &work->actor, Act, Die);
      GV_ActorEX( &work->actor );
      work->ctrl = ctrl;
      work->stream_h = ctrl->stream_h;
      work->type = type;

      if(LipStreamInit(work) < 0)
	{
	  GV_DestroyActor(work);
	  return NULL;
	}
    }

  works = work;
  return work;
}

/*
 * ドライバ登録
 */
#if 1 //BP_GCC
static GM_STREAM_DRIVER driver = {
   NULL, 0, NewStreamLipDriver
};
#else
static GM_STREAM_DRIVER driver = {
  driver: NewStreamLipDriver
};
#endif

int GM_StreamLipDriverInit(void)
{
  using_handle = 0;
  works = NULL;
  GM_StreamAddDriver(&driver, CHANK_TYPE_LIP);
  return 0;
}

/*
 * 外部関数
 */
LipInfo * GetLipInfo(void)
{
  static LipInfo dummy = {PHONE_sil, PHONE_sil, 0, 0, 0};
  if(works == NULL) return &dummy;
  return &works->info;
}
