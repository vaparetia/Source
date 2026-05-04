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
#include "g_struct.h"
#include "codec.h"

#include "libfs.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"
#include "fmt_demo.h"
#include "dmapack.h"
#include "def_dma.h"

#define _3d_util_c_
#include "3d_util.h"
#include "codecmem.h"
#include "codec_config.h"

#ifdef DEBUG
#define DBG(args...)   printf(args)
#else
#define DBG(args...)
#endif /* DEBUG */



typedef struct {
  FVECTOR root;
  FVECTOR joints[64];
  FVECTOR quat_buffer[4];
  FVECTOR vec;
  FMATRIX mat;
  FMATRIX root_mat;
  FMATRIX mats[64];
} ScrPadWork;


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

/* =======================================================================
 * 空間上に配置された EVM モデルにおける、指定した関節の座標を得る
 * ======================================================================= */
FVECTOR * get_joints_vector(FVECTOR * vec, DG_EVMOBJ * evmobj, int num)
{
  FMATRIX * now;
  FMATRIX * parent;
  FMATRIX * skel_mat;
  EVM_SKEL * skel;

  skel_mat = evmobj->matrix[ evmobj->use_buffer ];
  skel = evmobj->def->skeleton + num;
  parent = &skel_mat[ skel->parent ];
 
  now = &skel_mat[ num ];

  vec->vx = skel->rt_tx;
  vec->vy = skel->rt_ty;
  vec->vz = skel->rt_tz;
  vec->vw = 1.0F;


#ifdef DEBUG  
  /* disp_vec(vec);
     disp_mat(parent); */
#endif /* DEBUG */

  _sceVu0ApplyMatrix(vec, parent, vec);
  
  return vec;
}


/* ====================================================================
 * 指定されたワールド座標を、現在のカメラから視た際の 2D 座標に変換する
 * ==================================================================== */
int get_render_point(float * x, float * y, FVECTOR * vec, int chanl)
{
  FMATRIX * camera;    /* カメラマトリクス */
  FMATRIX * scrn;
  FVECTOR   tmp;
  
  int i;

  camera = &DG_Chanls[chanl].eye_pers2;
  if(!(GV_PadDataDirect[1].press & PAD_R1) ) return 0;
  scrn = &GM_PlayerBody->objs->objs[0].screen;
  printf("-----\n");
  disp_mat(scrn);

 return 0;
}

/*
 * 指定された回転角で、回転マトリクスを作成する(テスト用)
 */
FMATRIX * create_rotate_matrix_YXZ(FMATRIX * mat,
				   float heading, float pitch, float roll)
{
  float sin_h, cos_h;
  float sin_p, cos_p;
  float sin_r, cos_r;

  sin_h = vu0_Sin(heading); cos_h = vu0_Cos(heading);
  sin_p = vu0_Sin(pitch);   cos_p = vu0_Cos(pitch);
  sin_r = vu0_Sin(roll);    cos_r = vu0_Cos(roll);

  mat->m[0][0] = cos_h * cos_r - sin_h * cos_p * sin_r;
  mat->m[0][1] = cos_h * sin_r + sin_h * cos_p * cos_r;
  mat->m[0][2] = sin_h * sin_p;
  mat->m[0][3] = 0.0F;

  mat->m[1][0] =  sin_p * sin_r;
  mat->m[1][1] = -sin_p * cos_r;
  mat->m[1][2] =  cos_p;
  mat->m[1][3] = 0.0F;

  mat->m[2][0] =  sin_h * cos_r + cos_h * cos_p * sin_r;
  mat->m[2][1] =  sin_h * sin_r - cos_h * cos_p * cos_r;
  mat->m[2][2] = -cos_h * sin_p;
  mat->m[2][3] = 0.0F;

  mat->m[3][0] = 0.0F;
  mat->m[3][1] = 0.0F;
  mat->m[3][1] = 0.0F;
  mat->m[3][1] = 1.0F;

  return mat;
}

/*
 * 平行移動マトリクスを作成する
 */
FMATRIX * create_trans_matrix(FMATRIX * mat, float x, float y, float z)
{
  mat->m[0][0] = 1.0F;
  mat->m[0][1] = 0.0F;
  mat->m[0][2] = 0.0F;
  mat->m[0][3] = x;

  mat->m[1][0] = 0.0F;
  mat->m[1][1] = 1.0F;
  mat->m[1][2] = 0.0F;
  mat->m[1][3] = y;
  
  mat->m[2][0] = 0.0F;
  mat->m[2][1] = 0.0F;
  mat->m[2][2] = 1.0F;
  mat->m[2][3] = z;

  mat->m[3][0] = 0.0F;
  mat->m[3][1] = 0.0F;
  mat->m[3][2] = 0.0F;
  mat->m[3][3] = 1.0F;
  return mat;
}
