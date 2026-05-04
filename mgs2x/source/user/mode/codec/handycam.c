//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	handycam.c
	簡易手ブレシミュレーション

	2000/07/24	Y.Kira
	$Id: handycam.c,v 1.1.1.3 2002/11/19 11:45:02 Yoshizawa1 Exp $
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

#define _handycam_c_
#include "codecmem.h"
#include "handycam.h"
#include "codec_config.h"

#ifdef PSX2
#ifdef _DEBUG_
#define DBG(...)  printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* _DEBUG_ */
#endif
#ifdef KP_XBOX
#define DBG
#endif

#define F_PI   ((float)M_PI)    /* float 型の Pie 値 */

#define X_BODY_AMP   5.00F
#define X_BODY_FUNC(omega)   (vu0_Sin((omega) / 6.0F) * X_BODY_AMP + vu0_Sin(omega / 2) / 100.0F + 0.08F)

#define Y_BODY_AMP   9.00F
#define Y_BODY_FUNC(omega)   (vu0_Sin((omega) / 7.0F) * Y_BODY_AMP + vu0_Sin(omega / 2) / 100.0F + 0.08F)

#define PULSE_FUNC(omega)  ((vu0_Sin((omega) * 10) + vu0_Sin(omega)) * vu0_Cos((omega)*3)/3)

/*
  #define X_AMP 2.0F
  #define Y_AMP 1.25F
*/
#define X_AMP   4.0F
#define Y_AMP   2.5F

#define X_PULSE(omega)  (PULSE_FUNC(omega) * X_AMP)
#define Y_PULSE(omega)  (PULSE_FUNC(omega) * Y_AMP)

typedef struct {
  GV_ACT_EX    actor;

  FMATRIX      mat;    /* カメラ補正マトリクス   */

  FVECTOR      trans;  /* カメラ位置の微細な移動 */
  FVECTOR      rot;    /* カメラ角度の微細な移動 */
 
  float        x_body_action;   /* カメラマンの左右揺れをあらわす角速度 */
  float        x_pre_level;     /* 直前の身体左右レベル                 */
  int          x_pulse_len;

  float        y_body_action;   /* カメラマンの上下揺れをあらわす角速度 */
  float        y_pre_level;     /* 直前の身体上下レベル                 */
  int          y_pulse_len;

} Work;

FMATRIX * hcGetMatrix(void * p)
{
  Work * work = p;
  return &work->mat;
}

static void calc_matrix(Work * work)
{
  /* 平行移動、回転から、カメラマトリクス補正マトリクスを作成する */
  MT_QuatToMat(&work->mat, &work->rot);

  work->mat.m[3][0] = work->trans.vx;
  work->mat.m[3][1] = work->trans.vy;
  work->mat.m[3][2] = work->trans.vz;
}

static void Act(Work * work)
{
  float x_body_level, y_body_level;
#if 0 //BP_PS2
//#ifdef PSX2
  /* 初期化 */
  work->trans = (FVECTOR){0.0F, 0.0F, 0.0F, 1.0F};
  work->rot   = (FVECTOR){0.0F, 0.0F, 0.0F, 1.0F};
#endif
#ifdef KP_XBOX
  FVECTOR z = {0.0F, 0.0F, 0.0F, 1.0F};
  work->trans = z;
  work->rot = z;
#endif

  x_body_level = X_BODY_FUNC(work->x_body_action);
  y_body_level = Y_BODY_FUNC(work->y_body_action);

  work->x_body_action += (F_PI * TIME_BASE) / 300.0F;
  work->y_body_action += (F_PI * TIME_BASE) / 300.0F;

  work->trans.vx = x_body_level;
  work->trans.vy = y_body_level;

  if(work->x_pulse_len <= 0)
    if(((BP_PS2_rand() >> 16) & 63) == 0)
      work->x_pulse_len = 100 + (BP_PS2_rand() >> 16) % 20;

  if(work->y_pulse_len <= 0)
    if(((BP_PS2_rand() >> 16) & 63) == 0)
      work->y_pulse_len = 50 + (BP_PS2_rand() >> 16) % 20;

  if(work->x_pulse_len > 0)
    {
      work->trans.vx += X_PULSE(work->x_body_action * 0.8F);
      work->x_pulse_len -= TIME_BASE;
    }
  if(work->y_pulse_len > 0)
    {
      work->trans.vy += Y_PULSE(work->y_body_action * 0.8F);
      work->y_pulse_len -= TIME_BASE;
    }

  work->x_pre_level = x_body_level;
  work->y_pre_level = y_body_level;

  calc_matrix(work);
}

static void Die(Work * work)
{
}

static int GetResources(Work * work)
{
  work->x_body_action = (float)((BP_PS2_rand()>>16) % 100) / 50.0F * F_PI;
  work->y_body_action = (float)((BP_PS2_rand()>>16) % 100) / 50.0F * F_PI;
  work->x_pulse_len = 0;
  work->y_pulse_len = 0;
  return 0;
}

void * NewHandyCam(void)
{
  Work * work;

  OPERATOR();

  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);

  if(GetResources(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}
