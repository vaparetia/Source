//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  bomb_sensor.c
  爆弾センサー表示

  2001/02/28  Y.Kira
  2001/05/31  S.Kobayashi
  $Id: bomb_sensor.c,v 1.1.1.3 2002/11/19 11:43:55 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <eekernel.h>
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
#include <stdlib.h>

#include "libutl.h"
#include "def_dma.h"
#include "gameheader.h"
#include "radsprite.h"
#include "radsize.h"
#include "radar_config.h"
#include "radar_macro.h"

#define _bomb_sensor_c_
#include "bomb_sensor.h"
#include "bomb.h"

#define F_PI ((float)M_PI)

#ifdef PSX2
#ifdef DEBUG
#define DBG(...)  printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* DEBUG */
#else
#define DBG
#endif

typedef struct {
  GV_ACT_EX   actor;

  /* 爆弾センサーで使用するテクスチャの TRI ハンドル */
  int          tri_handle;

  /* 登録されている B_SENSOR 反応構造体のリスト */
  RADAR_BOMB * begin;
  RADAR_BOMB * end;

  /* レーダ中心のオブジェクト */
  SPR_OBJ    * radar_center;

  int          show_status;

  /* 親であるレーダ chara と同期が必要なデータ。
     それぞれ、親 chara の Work 内のデータに対するポインタで保持する。 */
  short * rot;
  char mode; // 爆弾？水中泡？ 
  
} Work;

static Work * now_work = NULL;


static int RecieveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;

  switch(signal)
    {
    case BOMB_MODE_SIGNAL:
#if 0
      {
	switch(value)
	  {
	  case ALERT_MODE_SNEAK:
	  case ALERT_MODE_SEARCH:
	    work->show_status = 1;
	    break;
	  case ALERT_MODE_ALERT:
	  case ALERT_MODE_AVOID:
	  case RAD_STATE_CHAFF:
	  case BOMB_FORCE_DISABLE:
	    work->show_status = 0;
	    break;
	  }
      }
#else
      work->show_status = value;	// 表示シグナル
#endif
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}


/*
 * RADAR_BOMB オブジェクトを作成
 */
RADAR_BOMB * BOMB_InitBomb(RADAR_BOMB * bomb)
{
  Work * work = now_work;
  int i;

  if(NULL == work) return NULL;

  /* 表示用スプライト生成 */
  bomb->empty = SPR_Create_2D_Object(SP_EMPTY, 4, work->radar_center);
  for(i = 0; i < BOMB_OBJ_NUMS; i++)
    {
      bomb->pre_rate[i] = 8192;
      bomb->cloud[i] = SPR_Create_2D_Object(SP_SPRITE_F, 4, bomb->empty);
#ifndef BOMB_NO_TEXTURE
      SPR_ObjSetTexture(bomb->cloud[i], BOMB_TEX, work->tri_handle);
      bomb->cloud[i]->ex_hd.tex.pabe = 0;
      bomb->texinfo[i].u = bomb->cloud[i]->ex_hd.tex.u;
      bomb->texinfo[i].v = bomb->cloud[i]->ex_hd.tex.v;
      bomb->texinfo[i].w = bomb->cloud[i]->ex_hd.tex.w;
      bomb->texinfo[i].h = bomb->cloud[i]->ex_hd.tex.h;
#endif /* BOMB_NO_TEXTURE */
    }
  
  bomb->pos = NULL;

  /*
   * センサー表示の色を初期化
   */
  bomb->r = BOMB_R;
  bomb->g = BOMB_G;
  bomb->b = BOMB_B;

 
  /* リストに追加 */
  bomb->next = NULL;
  bomb->prev = work->end;
  if(NULL != work->end)  work->end->next = bomb;
  work->end = bomb;
  if(NULL == work->begin) work->begin = bomb;
  return bomb;
}

/*
 * 指定された RADAR_BOMB 構造体を、リストから除去する
 */
void BOMB_Remove(RADAR_BOMB * bomb)
{
  Work * work = now_work;

  if(NULL == work) return;

  if(NULL != bomb->prev) bomb->prev->next = bomb->next;
  if(NULL != bomb->next) bomb->next->prev = bomb->prev;

  if(work->begin == bomb) work->begin = bomb->next;
  if(work->end == bomb) work->end = bomb->prev;

  /* 定義してある 2D オブジェクトを破棄 */
  SPR_Destroy_2D_Object(bomb->empty);  /* その子も道連れ */
}

/*
 * 爆弾センサ反応領域を設定する
 */
void BOMB_SetArea(RADAR_BOMB * bomb, FVECTOR * pos,
		  float cx, float cy, float width, float height)
{
  Work * work = now_work;


  if(NULL == work) return;

  /* センサー表示領域の位置とサイズを指定 */
  bomb->pos      = pos;
  bomb->center_x = cx;
  bomb->center_y = cy;
  bomb->width    = width;
  bomb->height   = height;

  /* 初期位相はランダム */
#ifndef PSX2
  bomb->rate = rand() % 4096;
#else
  bomb->rate = (BP_PS2_rand() >> 16) % 4096;
#endif
  /* empty を表示状態にして、表示を開始する */
  SPR_SHOW(bomb->empty);
}

/*
 * 爆弾の表示の明るさを指定する。 0 で完全に見えなくなる。
 */
void BOMB_SetBlight(RADAR_BOMB * bomb, int blight)
{
  int b = blight;

  if(b < 0) b = 0;
  if(b > BOMB_MAX_ALPHA) b = BOMB_MAX_ALPHA;
  bomb->blight = b;
}

static void rot_vec_XZ(FVECTOR *pos,FVECTOR *res ,int dir)
{
  SVECTOR rot;
  FVECTOR tmppos;

  tmppos = *pos ;
  tmppos.vy = 0.0F ;
  tmppos.vw = 0.0F ;
  rot.vx = 0; rot.vy = (dir)&4095; rot.vz = 0;
  DG_SetPos2( &DG_ZeroVector, &rot ) ;
  //	DG_PutVector( pos, res, 1 ) ;
  DG_PutVector( &tmppos, &tmppos, 1 ) ;
  res->vx = tmppos.vx ;
  res->vz = tmppos.vz ;
}

static int setup_bomb_object(RADAR_BOMB * bomb, FVECTOR * center, int dir)
{
  FVECTOR tmppos, zoom_rate;
  SPR_OBJ * obj = bomb->empty;
  SPR_POS pos;

  SPR_HIDE(obj);

  zoom_rate.vx = RADAR_ZOOM_RATE_W;
  zoom_rate.vy = zoom_rate.vw = 0.0F;
  zoom_rate.vz = RADAR_ZOOM_RATE_H;

  tmppos.vx = bomb->pos->vx - center->vx ;
  tmppos.vz = bomb->pos->vz - center->vz ;
  tmppos.vy = tmppos.vw = 0.0f ;
  rot_vec_XZ( &tmppos, &tmppos, dir);
  pos.x = tmppos.vx * _ABSf(zoom_rate.vx);
  pos.y = tmppos.vz * _ABSf(zoom_rate.vz);

  pos.x *= SC_RATE_W;/*2002.04.09yano*/
  // Arm fix:
  // fixed drifting of bomb cloud when moving around by multiplying by 0.75
  // fixes MGSTWO-2494
  pos.y *= SC_RATE_H * 0.75f;

  SPR_SetPosEmpty(obj, &pos);  /* 座標指定 */
  SPR_SHOW(obj);          /* 表示開始 */
  return 0;
}

/*
 * 爆弾表示スプライトのクリッピングを行う
 */
static void bomb_clip(RADAR_BOMB * bomb, int rate, SPR_OBJ * obj, float obj_w, float obj_h)
{
  float ax, ay;  /* 絶対座標 */
  int idx;
  float rdr_w, rdr_h;

  /* SPR_OBJ 上に記録されている
     座標値は爆弾位置からの相対座標なので、レーダ左上からの座標に変換する */
  ax = obj->sprite.pos.x;
  ay = obj->sprite.pos.y;

  /* この時点での座標は、スプライト座標系 */


  /* レーダー中央からの座標を計算する */
  ax += bomb->empty->empty.pos.x;
  ay += bomb->empty->empty.pos.y;

  /*
   * レーダー左上からの座標に変換する
   */

  /* レーダー左上のスプライト座標値の計算 */
  rdr_w = RADAR_WINDOW_W * 512.0 / DRAW_WIDTH ;
  rdr_h = RADAR_WINDOW_H * 384.0 / DRAW_HEIGHT ;

  ax += rdr_w / 2;
  ay += rdr_h / 2;

  obj->sprite.col.r = bomb->r;
  obj->sprite.col.g = bomb->g;
  obj->sprite.col.b = bomb->b;
  obj->sprite.col.a = rate * bomb->blight / BOMB_MAX_ALPHA;
  obj->head.alpha = SCE_GS_SET_ALPHA(0, 2, 0, 1, 0);
  obj->head.flags |= SPR_FLAG_ALPHA;

  SPR_SHOW(obj);

  /*
   * 完全にレーダの表示外となる場合は表示を禁止する
   */
  if (ax < -obj_w)
  {
     SPR_HIDE(obj);
     return;
  }
  if (ax >= rdr_w)
  {
     SPR_HIDE(obj);
     return;
  }
  if (ay < -obj_h)
  {
     SPR_HIDE(obj);
     return;
  }
  if (ay >= rdr_h)
  {
     SPR_HIDE(obj);
     return;
  }

  /* 現在の回転位置によってクリッピングすべき値が変化する */
  /* 回転方向によって、各値を入れ換える */

  if (ax < 0)
  {
     float w = obj->ex_hd.tex.w;
     float t = -ax / obj_w;  /* 左側を削る量 */
     obj->sprite.pos.x += t * obj_w;
     obj->sprite.dw -= t * obj_w;
     obj->ex_hd.tex.u += t * w;
     obj->ex_hd.tex.w -= t * w;
  }
  if (ax >= rdr_w - obj_w)
  {
     float w = obj->ex_hd.tex.w;
     float t = (ax + obj_w - rdr_w) / obj_w;  /* 右側を削る量 */
     obj->sprite.dw -= t * obj_w;
     obj->ex_hd.tex.w -= t * w;
  }
  
  if (ay < 0)
  {
     float h = obj->ex_hd.tex.h;
     float t = -ay / obj_h;  /* 上を削る量 */
     obj->sprite.pos.y += t * obj_h;
     obj->sprite.dh -= t * obj_h;
     obj->ex_hd.tex.v += t * h;
     obj->ex_hd.tex.h -= t * h;
  }
  if (ay >= rdr_h - obj_h)
  {
     float h = obj->ex_hd.tex.h;
     float t = (ay + obj_h - rdr_h) / obj_h;  /* 下を削る量 */
     obj->sprite.dh -= t * obj_h;
     obj->ex_hd.tex.h -= t * h;
  }
}

static void bomb_animation(RADAR_BOMB * bomb)
{
  int i;
  float width, height;
  float obj_w, obj_h;
  float cx, cy;
  SPR_POS pos;
  int rate, alpha = 0;
  float rad;

  /* 検知エリアの広さを求める */
  width  = (bomb->width * SC_RATE_W) * (float)RADAR_ZOOM_RATE_W;/*2002.04.08yano*/
  height = (bomb->height * SC_RATE_H) * (float)RADAR_ZOOM_RATE_H;
//printf("bomb->width %f, bomb->height %f\n",bomb->width,bomb->height);
//printf("width %f, height %f\n",width,height);

  /* オブジェクトの大きさは、検知エリアの縦横平均の 100% とする。 */
  obj_w = obj_h = (width + height) * 1.5F / 2;
//printf("objw %f, objh %f\n",obj_w,obj_h);

  cx = (bomb->center_x * SC_RATE_W) * RADAR_ZOOM_RATE_W;/*2002.04.08yano*/
  cy = (bomb->center_y * SC_RATE_H) * RADAR_ZOOM_RATE_H;
  for(i = 0; i < BOMB_OBJ_NUMS; i++)
    {
      rate = (bomb->rate + i * (4096 / BOMB_OBJ_NUMS)) & 4095;

      alpha =
		(int)(((-vu0_Cos((float)rate * F_PI / 2048.0F) + 1.0F) / 2.0F) *
			  (float)BOMB_MAX_ALPHA);
	  
      if(rate < bomb->pre_rate[i])  /* 一周した */
		{
			/* 表示位置を設定 */
#ifndef PSX2
			pos.x = cx + (rand() % (int)width) - obj_w / 2;
			pos.y = cy + (rand() % (int)height) - obj_h / 2;
#else
			pos.x = cx + ((BP_PS2_rand() >> 16) % (int)width) - obj_w / 2;
			pos.y = cy + ((BP_PS2_rand() >> 16) % (int)height) - obj_h / 2;
#endif
         bomb->bomb_obj_pos[i].x = pos.x;
         bomb->bomb_obj_pos[i].y = pos.y;
		}
      SPR_SetPosSprite(bomb->cloud[i], &bomb->bomb_obj_pos[i]);
      
      /* サイズを設定 */
      SPR_SetSizeSprite(bomb->cloud[i], obj_w, obj_h);

      /* クリッピングの前に、UV 値を復元する */
      {
         int rotateFlag = 0;
         bomb->cloud[i]->ex_hd.tex.u = bomb->texinfo[i].u;
         bomb->cloud[i]->ex_hd.tex.v = bomb->texinfo[i].v;
         bomb->cloud[i]->ex_hd.tex.w = bomb->texinfo[i].w;
         bomb->cloud[i]->ex_hd.tex.h = bomb->texinfo[i].h;

         // need to rotate!
         // TODO: update the SPRITE_F dmapack submission
         // also update the clipping to consider this
         switch (rand() & 3)
         {
         case 0:
            rotateFlag = SPR_FLAG_ROTATE_0;
            break;
         case 1:
            rotateFlag = SPR_FLAG_ROTATE_90;
            break;
         case 2:
            rotateFlag = SPR_FLAG_ROTATE_180;
            break;
         case 3:
            rotateFlag = SPR_FLAG_ROTATE_270;
            break;
         }
         bomb->cloud[i]->head.flags |= rotateFlag;
      }

      bomb_clip(bomb, alpha, bomb->cloud[i], obj_w, obj_h);
      bomb->pre_rate[i] = rate;
    }
  bomb->rate = (bomb->rate + BOMB_DELTA_SPEED) & 4095;
}

#if 0
	    if( GM_Configuration & MENU_RADAR_OFF ){
		  SPR_HIDE( bomb->empty );
    	}
#endif

static void Act(Work * work)
{
  RADAR_BOMB * bomb;
  FVECTOR center;
  int dir = *(work->rot);  /* 親の base_rot 値を得る */
#if 0
  if((GM_AlertMode == ALERT_MODE_ALERT) ||
     (GM_AlertMode == ALERT_MODE_AVOID) || 
	 ( GM_Configuration & MENU_RADAR_OFF ) ) // by koba4
    SPR_HIDE(work->radar_center);
  else
    {
      if(work->show_status)
	SPR_SHOW(work->radar_center);
      else
	SPR_HIDE(work->radar_center);
    }

  if((GM_CheckMenuStatus ( MENU_MODE_OFF)) ||
     (GV_PauseLevel & GV_LEVEL_NORMAL) )
    SPR_HIDE(work->radar_center);
#endif

  /* アイテムがスコープであれば表示しない */
  /*if(IT_Scope == PL_GetPlayerItem()) SPR_HIDE(work->radar_center); */
#if 0
  switch( work->mode ){
  case SK_MODE_BOMB : // bomb
	if(IT_BombSenserA != PL_GetPlayerItem()){
  		SPR_HIDE(work->radar_center);
	}
	break;
  default : break;
  }  
#endif
  /* レーダ中心と回転を求める */
  center = *(GM_RadarGetCenter());

  /*
   * 各爆弾の処理を行う
   */
  {
  	SPR_OBJ *obj;

  	for(bomb = work->begin; bomb != NULL; bomb = bomb->next){
	  obj = bomb->empty;
	  SPR_SHOW( obj );
	  switch( bomb->mode ){
	  case SK_MODE_BOMB : // bomb
	    if( ( work->show_status & RADAR_D_C4AREA ) == 0 ){
		  SPR_HIDE(obj);
		  continue;
	    }
	    break;
	  default :
	    if( ( work->show_status & RADAR_D_AIRAREA ) == 0 ){
		  SPR_HIDE(obj);
		  continue;
	    }
	    break;
	  }  
	  if(!setup_bomb_object(bomb, &center, dir)){
            bomb_animation(bomb);
	  }
	}
  }
}

static void Die(Work * work)
{
  RADAR_BOMB * bomb;
  RADAR_BOMB * next;

  bomb = work->begin;
  while(NULL != bomb)
    {
      next = bomb->next;
      BOMB_Remove(bomb);
      bomb = next;
    }
  SPR_Destroy_2D_Object(work->radar_center);

  /* テクスチャを開放 */
  SPR_KillTexture(work->tri_handle);
  now_work = NULL;
}

static int GetResources(Work * work, short * base_rot,
			SPR_OBJ * parent, float offset_y)
{
  if(NULL != now_work) return -1;
  now_work = work;

  DBG("BOMB Sensor Setup...\n");

  /* シグナルハンドラの設定 */
  GV_SetActorSignalFunc(work, RecieveSignal);

  /* 未登録状態に初期化する */
  work->begin = work->end = NULL;

  /* レーダマトリクスが格納される FMATRIX 構造体のポインタを保持 */
  work->rot = base_rot;

  /* 表示用に使用するテクスチャを読み込み、準備する */
#ifndef BOMB_NO_TEXTURE
  if((work->tri_handle = SPR_LoadTexture(BOMB_TRI)) < 0)
    {
      printf("texture not found.\n");
      HANGUP();
    }
#endif
  /* レーダ中心の基準となる 2D オブジェクトを生成 */
  if(NULL == (work->radar_center = SPR_Create_2D_Object(SP_EMPTY, 4, parent)))
    {
      DBG("could not create sprite.\n");
      SPR_KillTexture(work->tri_handle);
      return -1;
    }
  work->show_status = 1;  /* 初期状態では表示許可 */
  
  /* レーダ中心オブジェクトの座標を設定 */
  {
    SPR_POS pos;

    // BP FIX - adjusted position slightly
    pos.x = (float) RAD_BOMB_CX + 0.5f;
    pos.y = (float) RAD_BOMB_CY + offset_y - 2.0f;

    SPR_SetPosEmpty(work->radar_center, &pos);
    SPR_SHOW(work->radar_center);
  }

  DBG("BOMB Sensor Setup [success]\n");
  
  return 0;
}

void * NewRadarBomb(short * base_rot, SPR_OBJ * parent, float offset_y )
{
  Work * work;
  
  if(NULL ==(work = GV_NewActor(GV_ACTOR_USER, sizeof(Work)))) return NULL;

  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);
  if(GetResources(work, base_rot, parent, offset_y))
    {
      GV_DestroyActor(work);
      DBG("init failed.\n");
      return NULL;
    }
  return work;
}

