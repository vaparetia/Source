//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  face.c
  リアルタイムEVM表情制御マネージャ
  表示中の EVM モデルの表情を制御する。

  2001/02/14  Y.Kira
  $Id: face.c,v 1.1.1.3 2002/11/19 11:43:53 Yoshizawa1 Exp $
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

#define _face_c_
#include "face.h"

typedef struct {
  GV_ACT_EX    actor;

  FC_EVM_FACE * begin;
  FC_EVM_FACE * end;
  
} Work;

static Work * now_work = NULL;

/*
 * FC_EVM_FACE 構造体を、Actor のリンクに追加する
 */
static void regist_face(Work * work, FC_EVM_FACE * face)
{
  face->prev = work->end;
  face->next = NULL;
  if(NULL != work->end) work->end->next = face;
  work->end = face;
  if(NULL == work->begin) work->begin = face;
}

FC_EVM_FACE * FC_NewFaceControl(DG_EVMOBJ * evm, int name_id, int flag)
{
  FC_EVM_FACE * face;

  if(NULL == now_work) return NULL;
  if(NULL == evm) return NULL;
  if(NULL == (face = GV_Malloc(sizeof(FC_EVM_FACE)))) return NULL;

  face->evm       = evm;
  face->status    = 0;
  face->mtn_num   = -1;
  face->wait_time = 0;
  face->bias_time = 0;
  face->time_cnt  = 0;

  MT_InitFaceControl(&face->f_ctrl, name_id, flag);
  ASSERT(NULL != face->f_ctrl.header);
  regist_face(now_work, face);   /* Actor のリンクに登録 */
  return face;
}

int FC_ReleaseFaceControl(FC_EVM_FACE * face)
{
  Work * work = now_work;

  if(NULL == work) return -1;
  if(NULL == face) return -1;

  /* リンクの中から、自分を除去する */
  if(NULL != face->prev) face->prev->next = face->next;
  if(NULL != face->next) face->next->prev = face->prev;

  /* 自分が始端もしくは終端であれば、自分を除去した後の始端、終端に更新する */
  if(work->begin == face) work->begin = face->next;
  if(work->end == face) work->end = face->prev;

  /* 自分の領域を開放する */
  GV_Free(face);

  return 0;
}

/*
 * 表情の再生を指定する 
 */
int FC_ExecFaceAnim(FC_EVM_FACE * face, int id,
		    int wait_time, int bias_time)
{
  /* now_work が NULL の場合、face で示される領域は
     既に開放されている筈なので何もしない。 */
  if(NULL == now_work) return -1;
  if(NULL == face) return -1;

  face->wait_time = wait_time;
  face->bias_time = bias_time;
  face->time_cnt  = 0;    /* カウンタを初期化         */
  face->mtn_num   = id;   /* 再生待ちのモーション番号 */
  face->status    = 0;
  return 0;
}

/*
 * 指定されたワークのアニメーションを、強制的に終了する。
 * 現在再生中でなければ、ループを中断する
 */
int FC_StopFaceAnim(FC_EVM_FACE * face)
{
  /* now_work が NULL の場合、face で示される領域は
     既に開放されている筈なので何もしない。 */
  if(NULL == now_work) return -1;
  if(NULL == face) return -1;

  face->wait_time = -1;
  face->bias_time = -1;
  face->status    = 0;   /* コマンド待ち状態にする */
  return 0;
}

/*
 * 一つ分の evm を処理
 */
static int act_proc(Work * work, FC_EVM_FACE * face)
{
  enum {
    STAT_STOP,
    STAT_EXEC,
    STAT_WAIT,
    STAT_IDOL,
  };
  switch(face->status)
    {
    case STAT_IDOL:
      /* 関節情報の反映 */
      MT_PutFaceMotion(face->evm, &face->f_ctrl);
    case STAT_STOP:
      if(face->mtn_num < 0) break;
      face->status = STAT_EXEC;
      MT_SetFaceMotion(&face->f_ctrl, face->mtn_num);

    case STAT_EXEC:
      /* 顔モーションの再生 */
      MT_ActFaceMotion(&face->f_ctrl);

      /* 関節情報の反映 */
      MT_PutFaceMotion(face->evm, &face->f_ctrl);

      /* モーションが終了したら、繰り返し待ちまたは停止状態に移行する */
      if(!(face->f_ctrl.flag & MT_FACE_ACTIVE))
	{
	  if(face->wait_time < 0)  /* ワンショット */
	    {
	      face->mtn_num = -1;
	      face->status = STAT_IDOL;
	    }
	  else                     /* ループ       */
	    {
	      face->status = STAT_WAIT;
	      face->time_cnt = 0;
	      face->bias_var = (BP_PS2_rand() >> 16) % (face->bias_time + 1);
	    }
	}
      break;
    case STAT_WAIT:
      face->time_cnt += TIME_BASE;
      if(face->time_cnt >= (face->wait_time + face->bias_var))
	{
	  MT_SetFaceMotion(&face->f_ctrl, face->mtn_num);
	  face->status = STAT_EXEC;
	}
      break;
    }	

  return 0;
}

static void Act(Work * work)
{
  FC_EVM_FACE * face;

  /* 登録されているすべてのモデルに対し、現在の処理を行う */
  for(face = work->begin; face != NULL; face = face->next)
    act_proc(work, face);
}

static void Die(Work * work)
{
  FC_EVM_FACE *face, *next;

  /* 登録されているワークすべてを開放する */
  face = work->begin;
  while(NULL != face)
    {
      next = face->next;
      FC_ReleaseFaceControl(face);
      face = next;
    }
  /* 最後に開放 */
  if(now_work == work) now_work = NULL;
}

static int GetResources_forProg(Work * work)
{
  if(now_work != NULL) return -1;

  /* 初期状態では、何も登録されていない */
  work->begin = work->end = NULL;

  now_work = work;
  return 0;
}

void * NewFaceManager_forProg(void)
{
  Work * work;

  if(NULL == (work = GV_NewActor(ACT_LEVEL, sizeof(Work)))) return NULL;

  GV_SetActor(work, Act, Die);
  GV_ActorEX(&(work->actor));
  if(GetResources_forProg(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}

static int GetResources(Work * work, int name, int where)
{
  return GetResources_forProg(work);
}

void * NewFaceManager(int name, int where)
{
  Work * work;

  if(NULL == (work = GV_NewActor(ACT_LEVEL, sizeof(Work)))) return NULL;

  GV_SetActor(work, Act, Die);
  GV_ActorEX(&(work->actor));
  if(GetResources(work, name, where))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}

