//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  outrange.c
  ハリアーなど、レーダーの範囲外に移動する対象を表示する。

  2001/04/26  Y.Kira
  $Id: outrange.c,v 1.1.1.3 2002/11/19 11:43:55 Yoshizawa1 Exp $
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

#define _outrange_c_
#include "sprite_2d.h"
#include "outrange.h"
#include "bomb.h"
#include "bomb_sensor.h"
#include "radar_config.h"
#include "radar_macro.h"

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
  GV_ACT_EX     actor;

  SPR_OBJ     * base_center;

  int           show_status;

  short       * rot;   /* 親の回転角 */
  FVECTOR	  * zoom_rate;
} Work;

static Work * now_work = NULL;

int RDR_InitOutrangeTarget(OUTRANGE_CTRL * orange,
			   FVECTOR * mov, int flag, int map)
{
  Work * work = now_work;
  int i;

  GM_InitRadarControl((RADAR_CTRL *)orange, mov, flag | RADAR_OUTRANGE, map);
//  orange->arrow = SPR_Create_2D_Object(SP_LINESTRIP, 4, work->base_center); by koba4
  orange->arrow = SPR_Create_2D_Object(SP_TRIANGLE, 4, work->base_center);
  ASSERT(NULL != orange->arrow);
  //  i = SPR_SetLineStripVertexNumber(orange->arrow, 3);
  //  ASSERT(i >= 0);
  for(i = 0; i < 4; i++){
	SPR_SetColorTriangle( orange->arrow, i, 180, 182, 180, 72);
  }
//    SPR_SetColorLineStrip(orange->arrow, i, OR_ARROW_R, OR_ARROW_G, OR_ARROW_B, 128);
  return 0;
}

#if 0
  SPR_SetColorTriangle( obj , 0, 180 , 182 , 180 , 72 );
  SPR_SetColorTriangle( obj , 1, 180 , 182 , 180 , 72 );
  SPR_SetColorTriangle( obj , 2, 180 , 182 , 180 , 72 );
#endif 

int RDR_FinishOutrangeTarget(OUTRANGE_CTRL * orange)
{
  SPR_Destroy_2D_Object(orange->arrow);
  GM_FreeRadarControl((RADAR_CTRL *)orange);
  return 0;
}

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
	work->show_status = value;
#endif
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
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

static void outrange_disp(FVECTOR * center, FVECTOR *zoom, OUTRANGE_CTRL * ctrl, int dir)
{
  FVECTOR tmppos;
  FVECTOR zoom_rate;
  FVECTOR fvtmp;
  static FVECTOR color[ 2 ] = { { 250.0f , 30.0f , 30.0f , 100.0f } ,
					     { 160.0f , 0.0f , 0.0f , 90.0f } };
  SPR_OBJ *obj = ctrl->arrow;
  SPR_POS pos;
  SPR_POS verts[3];  /* LineStrip の頂点バッファ */
  int outrange = 0;

  /* 一旦非表示に */
  SPR_HIDE(obj);

  /* レーダ中心からの表示座標を求める */
#if 0
  zoom_rate.vx = RADAR_ZOOM_RATE_W;
  zoom_rate.vy = zoom_rate.vw = 0.0F;
  zoom_rate.vz = RADAR_ZOOM_RATE_H;
#endif

  zoom_rate = *zoom;
  if(dir) rot_vec_XZ(&zoom_rate, zoom, dir & 4095);
  rot_vec_XZ(ctrl->radar.pos, &tmppos, dir);

  pos.x = (float)(tmppos.vx - center->vx) * _ABSf(zoom_rate.vx);
  pos.y = (float)(tmppos.vz - center->vz) * _ABSf(zoom_rate.vz);

//  pos.y *= SC_RATE_H;

  /*
   * レーダ範囲からのはみ出しをチェックする。
   * 画面外にあると判断された場合、画面外表示の表示先を計算する。
   */
  {

	if ( _ABSf( pos.x ) >= ( RADAR_WINDOW_W / 2 ) || _ABSf( pos.y ) >= ( RADAR_WINDOW_H / 2 ) ){
	    outrange = 1;   /* はみ出しフラグ */

		fvtmp = DG_ZeroVector;
		fvtmp.vx = pos.x;
		fvtmp.vy = pos.y;
		_sceVu0Normalize( &fvtmp , &fvtmp );
		pos.x = fvtmp.vx * ( SK_RADAR_WINDOW_W / 2 ) * SC_RATE_W;/*2002.04.08yano*/
		pos.y = fvtmp.vy * ( SK_RADAR_WINDOW_H / 2 ) * SC_RATE_H;
#if 0
		printf("x = %f\n" , pos.x );
		printf("y = %f\n" , pos.y );
#endif
	}
  }
#if 0
  /* 左右からのはみ出しをチェック */
  if(_ABSf(pos.x) >= (RADAR_WINDOW_W / 2))
    {
      float x = _ABSf(pos.x);

      outrange = 1;   /* はみ出しフラグ */

      /* 中央からターゲットまでを結ぶ線分を、
	 レーダの左右で切った場合の y 座標を求め、pos.y に再設定する */
      pos.y = pos.y * (RADAR_WINDOW_W / 2) / x;
      pos.x = (RADAR_WINDOW_W / 2 - 1) * (pos.x / x);
    }
  
  /* 上下からのはみ出しをチェック */
  if(_ABSf(pos.y) >= (RADAR_WINDOW_H * SC_RATE_H / 2))
    {
      float y = _ABSf(pos.y);

      outrange = 1;  /* はみ出しフラグ */

      /* 中央からターゲットまでを結ぶ線分を、
	 レーダの上下で切った場合の x 座標を求め、pos.x に再設定する。 */
      pos.x = pos.x * (RADAR_WINDOW_H * SC_RATE_H / 2) / y;
      pos.y = (RADAR_WINDOW_H * SC_RATE_H / 2 - 1) * (pos.y / y);
    }
#endif
  /* 計算された値がレーダ内にあれば、通常の表示で
     間に合っているはずなので、何もしない。 */
  if(!outrange) return;

  /*
   * この時点では、pos は画面外表示の表示位置を指しているので、
   * 画面外表示 LineStrip の頂点リストを作成する
   */
  verts[1] = pos;  /* 中央の点こそが、矢印の表示位置となる。 */

  /*
   * 矢印の両端の座標を計算する
   */
  {

    float x, y;  /* 両端の点を結ぶ線分と、原点と目標点を結ぶ線分の交点 */

    x = pos.x * 0.8F;  /* 全体の長さの 80% の位置に置く */
    y = pos.y * 0.8F;  /* 全体の長さの 80% の位置に置く */
#if 0
    verts[0].x = x - pos.y * 0.05F;
    verts[0].y = y + pos.x * 0.05F;

    verts[2].x = x + pos.y * 0.05F;
    verts[2].y = y - pos.x * 0.05F;
#else
	fvtmp.vx *= ( SK_RADAR_WINDOW_W / 2 );
	fvtmp.vy *= ( SK_RADAR_WINDOW_W / 2 );
    verts[0].x = x - ( fvtmp.vy * 0.05F ) * SC_RATE_W;/*2002.04.08yano*/
    verts[0].y = y + ( fvtmp.vx * 0.05F ) * SC_RATE_H;

    verts[2].x = x + ( fvtmp.vy * 0.05F ) * SC_RATE_W;/*2002.04.08yano*/
    verts[2].y = y - ( fvtmp.vx * 0.05F ) * SC_RATE_H;
#endif
  }

  /* LineStrip の頂点を設定 */
//  SPR_SetPosLineStrip(ctrl->arrow, 0, 3, verts);

  SPR_SetColorTriangle( obj , 1, ( int )color[ 0 ].vx , ( int )color[ 0 ].vy , ( int )color[ 0 ].vz , ( int )color[ 0 ].vw );
  SPR_SetColorTriangle( obj , 0, ( int )color[ 1 ].vx , ( int )color[ 1 ].vy , ( int )color[ 1 ].vz , ( int )color[ 1 ].vw );
  SPR_SetColorTriangle( obj , 2, ( int )color[ 1 ].vx , ( int )color[ 1 ].vy , ( int )color[ 1 ].vz , ( int )color[ 1 ].vw );
  SPR_SetColorTriangle( obj , 3, ( int )color[ 1 ].vx , ( int )color[ 1 ].vy , ( int )color[ 1 ].vz , ( int )color[ 1 ].vw );

  SPR_SetPosTriangle( obj , verts );
  SPR_SHOW(ctrl->arrow);  /* 可視に */
}


static void Act(Work * work)
{
  RADAR_CTRL * rdr;
  FVECTOR center;
  int     dir = *(work->rot);

#if 0
  if((GM_AlertMode == ALERT_MODE_ALERT) ||
     (GM_AlertMode == ALERT_MODE_AVOID))
    SPR_HIDE(work->base_center);
  else
    {
      if(work->show_status)
	SPR_SHOW(work->base_center);
      else
	SPR_HIDE(work->base_center);
    }

  if((GM_CheckMenuStatus ( MENU_MODE_OFF)) ||
     (GV_PauseLevel & GV_LEVEL_NORMAL))
    SPR_HIDE(work->base_center);
#endif

//  if(IT_BombSenserA != PL_GetPlayerItem()) SPR_HIDE(work->base_center);


  if( work->show_status & RADAR_D_OUTDIR ){
      SPR_SHOW(work->base_center);
  } else {
      SPR_HIDE(work->base_center);
      return;
  }
  

  /* レーダー中心とするベクトルを得る */
  if(dir)
    rot_vec_XZ(GM_RadarGetCenter(), &center, dir);
  else
    center = *(GM_RadarGetCenter());

  for(rdr = GM_RadarControlGetTop(); rdr != NULL; rdr = rdr->next)
    {
      /* アウトレンジ属性がついていないものは処理対象としない */
      if(!(rdr->flag & RADAR_OUTRANGE)) continue;
      outrange_disp(&center, work->zoom_rate, (OUTRANGE_CTRL *)rdr, dir);
    }
#if 0
  /* アイテムがスコープであれば表示しない */
  if(IT_Scope == PL_GetPlayerItem()){
	SPR_HIDE( work->base_center );
  }
#endif
}

static void Die(Work * work)
{
  SPR_Destroy_2D_Object(work->base_center);
  if(now_work == work) now_work = NULL;
}

static int GetResources(Work * work,
			short * base_rot,
			FVECTOR *zoom_rate,
			SPR_OBJ * parent,
			float offset_y)
{
  SPR_POS pos;

  if(NULL != now_work) return -1;
  now_work = work;

  GV_SetActorSignalFunc(work, RecieveSignal);

  work->rot = base_rot;

  work->base_center = SPR_Create_2D_Object(SP_EMPTY, 4, parent);
  ASSERT(NULL != work->base_center);
  work->show_status = 1;


  pos.x = RAD_BOMB_CX;
  pos.y = RAD_BOMB_CY + offset_y;

  work->zoom_rate = zoom_rate;
  
  SPR_SetPosEmpty(work->base_center, &pos);
  SPR_SHOW(work->base_center);

  return 0;
}

void * NewRadarOutRangeTarget(short * base_rot, FVECTOR *zoom_rate, SPR_OBJ * parent, float offset_y)
{
  Work * work;

  if(NULL == (work = GV_NewActor(GV_ACTOR_USER, sizeof(Work)))) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, base_rot, zoom_rate, parent, offset_y))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}
