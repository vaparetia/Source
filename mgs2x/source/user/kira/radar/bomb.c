//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  bomb.c
  爆弾センサー検知エリアの設定

  2001/03/09 Y.Kira

  $Id: bomb.c,v 1.2 2002/11/29 17:04:46 takaki Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <math.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "libhzx.h"
#include "def_dma.h"
#include "gameheader.h"
#include "radsprite.h"
#include "radsize.h"

#include "bomb.h"
#include "bomb_sensor.h"
#include "bomb_child.h"

#define DEFAULT_CHANGE_TIME 60

/* 子 Actor の構造体を、そのまま描画済領域情報として使用する */
typedef struct _REGION_INFO {
  GV_ACT_EX  actor;

  struct _REGION_INFO * next;
  HZX_BLOCK * blk;    /* ブロック構造体へのポインタ */
  HZX_TRP   * trp;    /* トラップ構造体へのポインタ */

} _REGION_INFO;

typedef enum {
  BL_BEGIN,   /* 現れる */
  BL_CONT,    /* 表示中 */
  BL_END,      /* 消える */
  BL_ALP_CHG, /* 明るさ変更 */	
} BLIGHT_STEP;

typedef struct { 
  GV_ACT_EX   actor;

  _REGION_INFO * info;

  BLIGHT_STEP    step;
  int            blight;   /* 明るさ係数             */
  int            change_time;  /* 明るさ変化に要する時間 */
  int            cnt_time;     /* 変化中の経過時間       */

  int            name;     /* 名称 */
  
  float			 change_alpha;		/* アルファを一時的に変更する係数 */
  float			 original_alpha;	/* メッセージが送られる前のchange_alphaの値 */
  float			 current_alpha;		/* 現在のアルファ値 */
  int            change_time_alp;  	/* アルファ変更:明るさ変化に要する時間 */
  int            cnt_time_alp;     	/* アルファ変更:変化中の経過時間       */
  int			 alp_change_flag;	/* アルファ変更している間に立っているフラグ */
} Work;


/*
 * 爆弾センサ表示消去プログラムインタフェース
 * ※シナリオからは、メッセージが使用できる
 */
void BOMB_Erase(Work * work, int d_time)
{
  work->cnt_time = 0;
  if(d_time >= 0) work->change_time = d_time;
  else work->change_time = DEFAULT_CHANGE_TIME;
  work->step = BL_END;
}

static void Act(Work * work)
{
  int change = 0;
  GV_MSG * msg;
  int n_msg;
  /*
   * シナリオ用に、メッセージでもぼわっと消せるようにしておく。
   */
  if(work->name >= 0)
    if(n_msg = GV_ReceiveMessage(work->name, &msg))
      while(n_msg > 0)
	{
	  switch(msg->message[0])
	    {
		  case 0:  /* 表示を効果付で消去する */
			work->cnt_time = 0;
			work->change_time = msg->message[1];
			if(work->change_time < 0)
			  work->change_time = DEFAULT_CHANGE_TIME;
			work->step = BL_END;
			break;
		  case 1: /* 色を変更する */
			{
				int alpha;
				work->original_alpha = work->current_alpha;
				work->cnt_time_alp = 0;
				alpha = msg->message[1];
				if( alpha > 255 ){
					alpha = 255;
				}
				if( alpha < 0 ){
					alpha = 0;
				}
				work->change_alpha =  alpha / 255.0f;
				work->change_time_alp = msg->message[2];
				if(work->change_time_alp < 0)
				  work->change_time_alp = DEFAULT_CHANGE_TIME;
				printf("change_alpha %f::change_time_alp::%d\n",work->change_alpha,work->change_time_alp);
				work->alp_change_flag = 1;
				break;
			}
	    }
	  n_msg--;
	  msg++;
  }
  
  switch(work->step)
    {
    case BL_BEGIN:
      work->cnt_time += TIME_BASE;
      work->blight = work->cnt_time * BOMB_MAX_ALPHA / work->change_time;
      if(work->blight > BOMB_MAX_ALPHA) work->blight = BOMB_MAX_ALPHA;
      change = 1;
      if(work->cnt_time >= work->change_time) work->step = BL_CONT;
      break;
    case BL_CONT:
      /* 表示中は何もしない */
      break;
    case BL_END:
      work->cnt_time += TIME_BASE;
      work->blight = BOMB_MAX_ALPHA -
		work->cnt_time * BOMB_MAX_ALPHA / work->change_time;
      if(work->blight < 0) work->blight = 0;
      change = 1;
      /* 終りなら死を宣告する */
      if(work->cnt_time >= work->change_time)
		{
			GV_DestroyActor(work);
			return;
		}
      break;
  }
  if( work->alp_change_flag == 1 ){
	  /* アルファ変更の移行中 */
	  int blight;
	  float rate, rate2;
	  float t;
	  if( work->change_time_alp < TIME_BASE ){
		  t = 1.0f;
	  } else {
		  t = (float)work->cnt_time_alp / work->change_time_alp;/* 0.0f～1.0f */
	  }
	  work->cnt_time_alp += TIME_BASE;
	  rate = work->change_alpha;
	  rate2 = ( work->original_alpha*(1-t)*(1-t) + rate*t*t + 2.0f*0.5f*t*(1-t) ); 
	  work->current_alpha = rate2;
	  blight = (int)( work->blight * rate2 );
	  GV_CallChildSignalFunc(work, BOMB_BLIGHT_SIGNAL, blight);
	  if(work->cnt_time_alp >= work->change_time_alp) {
		  work->original_alpha = work->current_alpha;
		  work->alp_change_flag = 0;
	  }
	  return;
  } else {
	  if( work->step != BL_CONT ){
		  int	blight;
		  blight = (int)( work->blight * work->original_alpha );
		  GV_CallChildSignalFunc(work, BOMB_BLIGHT_SIGNAL, blight);
		  return;
	  }
  }

  if(!change) return;
  /*
   * 明るさに変更があった場合は、子 Actor に、明るさ表示のシグナルを送る
   */
  GV_CallChildSignalFunc(work, BOMB_BLIGHT_SIGNAL, work->blight);
}

static void Die(Work * work)
{
  /* こいつが死ねば子も死ぬ */
}

/*
 * 指定された名称のトラップ全てに、センサー表示を行う
 */
static int setup_bomb_fog(Work * work, int name, int r, int g, int b , char mode )
{
  _REGION_INFO * info;
  HZX_GROUP_ID hzx_id;
  HZX_BLOCK * blk;
  HZX_TRP   * trp;
  FVECTOR     p1, p2;
  FVECTOR     s1, s2;
  extern int	HZX_GetNextTrap( HZX_BLOCK **blkp, HZX_TRP **trpp );

  /* ぼわっと現れる用意 */
  work->blight = 0;
  work->step = BL_BEGIN;
  work->change_time = DEFAULT_CHANGE_TIME;
  work->cnt_time = 0;

  /*
    float width, height;
    float fx, fy, fz;
    float cx, cy;
    
    width = -1.0F;
    height = -1.0F;
    fx = fy = fz = 0.0F;
    cx = cy = 0.0F;
  */

  hzx_id = 0;
  HZX_FindTrap(hzx_id, name, &blk, &trp);

  work->info = NULL;
#ifndef KP_WINDOWS
  do {
    ASSERT(trp != NULL);
    /*
     * 過去に配置したトラップと同じ領域であれば、子 Actor は起動されない
     */
    p1.vx = blk->tx + trp->b1.vx;
    p1.vy = blk->ty + trp->b1.vy;
    p1.vz = blk->tz + trp->b1.vz;

    s1.vx = trp->b2.vx - trp->b1.vx;
    s1.vy = trp->b2.vy - trp->b1.vy;
    s1.vz = trp->b2.vz - trp->b1.vz;

    for(info = work->info; info != NULL; info = info->next)
      {
	p2.vx = info->blk->tx + info->trp->b1.vx;
	p2.vy = info->blk->ty + info->trp->b1.vy;
	p2.vz = info->blk->tz + info->trp->b1.vz;
	
	s2.vx = info->trp->b2.vx - info->trp->b1.vx;
	s2.vy = info->trp->b2.vy - info->trp->b1.vy;
	s2.vz = info->trp->b2.vz - info->trp->b1.vz;

	if((p1.vx == p2.vx) && (p1.vx == p2.vx) && (p1.vx == p2.vx) &&
	   (s1.vx == s2.vx) && (s1.vx == s2.vx) && (s1.vx == s2.vx))
	  break;
      }
    
    /* info が NULL であれば、同一の領域を指しているトラップは無い */
    if(info == NULL)
      { 
    	info = NewBombAreaChild(blk, trp, r, g, b , mode );
	info->next = work->info;
	work->info = info;
	GV_SetActorChild(work, info);
      }
  } while(HZX_GetNextTrap(&blk, &trp));
#else
  if( trp ) {	// 何故かボムが無いステージがあるのでエラー対策(By Takaki)
  do {
    ASSERT(trp != NULL);
    /*
     * 過去に配置したトラップと同じ領域であれば、子 Actor は起動されない
     */
    p1.vx = blk->tx + trp->b1.vx;
    p1.vy = blk->ty + trp->b1.vy;
    p1.vz = blk->tz + trp->b1.vz;

    s1.vx = trp->b2.vx - trp->b1.vx;
    s1.vy = trp->b2.vy - trp->b1.vy;
    s1.vz = trp->b2.vz - trp->b1.vz;

    for(info = work->info; info != NULL; info = info->next)
      {
	p2.vx = info->blk->tx + info->trp->b1.vx;
	p2.vy = info->blk->ty + info->trp->b1.vy;
	p2.vz = info->blk->tz + info->trp->b1.vz;
	
	s2.vx = info->trp->b2.vx - info->trp->b1.vx;
	s2.vy = info->trp->b2.vy - info->trp->b1.vy;
	s2.vz = info->trp->b2.vz - info->trp->b1.vz;

	if((p1.vx == p2.vx) && (p1.vx == p2.vx) && (p1.vx == p2.vx) &&
	   (s1.vx == s2.vx) && (s1.vx == s2.vx) && (s1.vx == s2.vx))
	  break;
      }
    
    /* info が NULL であれば、同一の領域を指しているトラップは無い */
    if(info == NULL)
      { 
    	info = NewBombAreaChild(blk, trp, r, g, b , mode );
	info->next = work->info;
	work->info = info;
	GV_SetActorChild(work, info);
      }
  } while(HZX_GetNextTrap(&blk, &trp));
  }
#endif

  work->original_alpha = 1.0f;

  printf("------------------------+++++++++++++++++++\n");
  return 0;
}


static int GetResources(Work * work, int where)
{
  int    name;
  int r, g, b;
  char   mode; // by koba4

  /* トラップから反応領域の位置とサイズを取得する */
  if(NULL == GCL_GetOption('t'))  /* "trap" */
    return -1;
  name = GCL_GetNextInt();  /* トラップの名称(?) */

  r = g = b = -1;
  if(NULL != GCL_GetOption('r'))  /* "rgb" */
    {
      r = GCL_GetNextInt();
      g = GCL_GetNextInt();
      b = GCL_GetNextInt();
    }

  mode = GCL_GetOptionValue( 'm' , SK_MODE_BOMB ); // mode  default sk_mode_bomb
  return setup_bomb_fog(work, name, r, g, b , mode );
}

static int GetResourcesP(Work * work, int name, int r, int g, int b)
{
  return setup_bomb_fog(work, name, r, g, b , SK_MODE_BOMB );
}

void * NewBombArea(int name, int where)
{
  Work * work;

  if(NULL == (work = GV_NewActor(GV_ACTOR_USER, sizeof(Work)))) return NULL;
  work->name = name;
  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));
  if(GetResources(work, where))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}

void * NewBombAreaP(int name, int r, int g, int b)
{
  Work * work;

  if(NULL == (work = GV_NewActor(GV_ACTOR_USER, sizeof(Work)))) return NULL;
  work->name = -1;
  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));
  if(GetResourcesP(work, name, r, g, b))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}
