/*
	partsub2.h
		パーティクルのシミュレーション

	2000/5/25 K.Kano
	$Id: partsub2.h,v 1.1.1.3 2002/11/19 11:43:27 Yoshizawa1 Exp $
*/


#ifndef _partsub2_h_
#define _partsub2_h_


#include <sys/types.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"


#define N_SIZE_TIMING		6
#define N_RGBA_TIMING		6
#define N_SIMULATION_FRAME	6
#define N_AFTERIMAGE		6


typedef unsigned int Ulong;


typedef struct {
	float vx,vy,vz,vw;
} FVECTOR_;


/* ツールからの受取データ */
typedef struct _DATA_PARTICLES {
    int version;
    int type;
    int size;

    int tex_strcode;
    int particle_strcode;

    Ulong flag;

    int simulation_frame_flag;
    int simulation_frame[N_SIMULATION_FRAME];
    int nums_simulation_frame[N_SIMULATION_FRAME];
    float num_percent;

    int born_type;
    float born_radius;
    float born_rnd_radius;

    float distance;
    float rnd_distance;

    int alive_time;
    int rnd_alive_time;

    int ending_time;
    int rnd_ending_time;

    float air_parcent;
    float gravity_parcent;
    float v_regist;

    float init_v_angle;
    FVECTOR_ init_v;
	float init_vrnd;

    int sz_flag;
    int sz_frame[N_SIZE_TIMING];
    FVECTOR_ sz[N_SIZE_TIMING];

    int rgba_flag;
    int rgba_frame[N_RGBA_TIMING];
    CVECTOR rgba[N_RGBA_TIMING];

	int alpha_mode;


    int late_frames;

    int afterimage_flag;
    int afterimage_frame[N_AFTERIMAGE];
    int afterimage_endframe[N_AFTERIMAGE];

    FVECTOR_ afterimage_sz_ratio[N_AFTERIMAGE];
    FVECTOR_ afterimage_rgba_ratio[N_AFTERIMAGE];
    FVECTOR_ afterimage_sz[N_AFTERIMAGE];
    CVECTOR afterimage_rgba[N_AFTERIMAGE];
} DATA_PARTICLES;


enum {
    PARTICLE_FLAG_DISTANCE=0,
    PARTICLE_FLAG_RND_DISTANCE,
    PARTICLE_FLAG_ALIVE_TIME,
    PARTICLE_FLAG_RND_ALIVE_TIME,

    PARTICLE_FLAG_ENDING_TIME,
    PARTICLE_FLAG_RND_ENDING_TIME,
    PARTICLE_FLAG_AIR,
    PARTICLE_FLAG_GRAVITY,

    PARTICLE_FLAG_V_REGIST,
    PARTICLE_FLAG_INIT_V,
    PARTICLE_FLAG_LOOP_TERM,
};


#define FLAG_TST(x,i)		(((x) & (1<<(i)))!=0)
#define FLAG_SET(x,i)		((x)|=(1<<(i)))
#define FLAG_RES(x,i)		((x)&=~(1<<(i)))
#define FLAG_REV(x,i)		((x)^=(1<<(i)))
#define FLAG_ISET(x,i,a)	((a) ? FLAG_SET(x,i) : FLAG_RES(x,i))


enum {
    PARTICLE_STATUS_NONE=0,
    PARTICLE_STATUS_ON_SEQUENCE,
    PARTICLE_STATUS_OFF_SEQUENCE,
    PARTICLE_STATUS_AFTERIMAGE,
};

#define MAX_MEMORY_FRAMES	16


typedef struct _SPR_PARTICLE {
    struct _SPR_PARTICLE *prea,*next;

	int status;
	int frame;
	int count;
	int afterimg_index;
    int mem_frames_index;

    float enable_distance;

	FVECTOR p;
	FVECTOR v;

	FVECTOR oldp[MAX_MEMORY_FRAMES];
	FVECTOR init_p;
	FVECTOR init_v;

	FVECTOR sz;
	FVECTOR vsz;

	FVECTOR oldsz[MAX_MEMORY_FRAMES];
	FVECTOR init_sz;
	FVECTOR init_vsz;

    FVECTOR rot;
    FVECTOR vrot;

    FVECTOR oldrot[MAX_MEMORY_FRAMES];
    FVECTOR init_rot;
    FVECTOR init_vrot;

	CVECTOR rgba;
	CVECTOR vrgba;

	CVECTOR oldrgba[MAX_MEMORY_FRAMES];
	CVECTOR init_rgba;
	CVECTOR init_vrgba;
} SPR_PARTICLE;


enum {
    PARTICLE_TYPE_ENDMARK=-1,

    PARTICLE_TYPE_NONE=0,
    PARTICLE_TYPE_SPRITE,
    PARTICLE_TYPE_LINE,
    PARTICLE_TYPE_POLY,
    PARTICLE_TYPE_POLY_DIAMOND,
};


enum {
    PARTICLE_ALPHAMODE_S=0,
    PARTICLE_ALPHAMODE_D,

    PARTICLE_ALPHAMODE_SAs,
    PARTICLE_ALPHAMODE_S1MAs,
    PARTICLE_ALPHAMODE_DAs,
    PARTICLE_ALPHAMODE_D1MAs,

    PARTICLE_ALPHAMODE_S_D,
    PARTICLE_ALPHAMODE_SAs_D,
    PARTICLE_ALPHAMODE_S1MAs_D,
    PARTICLE_ALPHAMODE_S_DAs,
    PARTICLE_ALPHAMODE_S_D1MAs,

    PARTICLE_ALPHAMODE_SAs_DAs,
    PARTICLE_ALPHAMODE_SAs_D1MAs,
    PARTICLE_ALPHAMODE_S1MAs_DAs,
    PARTICLE_ALPHAMODE_S1MAs_D1MAs,

    PARTICLE_ALPHAMODE_S_MDAs,
    PARTICLE_ALPHAMODE_MSAs_D,
};


/* 共通 */
typedef struct {
	int type;
	int size;
} COM_PARTICLES;


typedef struct {
	COM_PARTICLES com;

	DG_PRIM2 *prim;
	DG_TEX *tex;

	SPR_PARTICLE *particle;

    SPR_PARTICLE *seq_link;
    SPR_PARTICLE *noseq_link;

	/* 発生位置 */
	FMATRIX			*tmat;
	FVECTOR			pos;

	/* 初期速度を入力された場合 */
	FVECTOR			init_v;

	/* 初期速度に関するフラグ
	   0 ... dataのinit_vを使用する/ワールド、      1 ... dataのinit_vを使用する/ローカル
	   2 ... この構造体のinit_vを使用する/ワールド、3 ... この構造体のinit_vを使用する/ローカル */
	int				init_v_flag;


	/* パラメータ */
    int frame;
    int count;
    int num_particles_on_frame,count_particles_on_frame;

	DATA_PARTICLES *data;
} SPR_PARTICLES;


/* parload.c */
int PARLoader(void *buf,int id);

/* partsub2.c */
void RestartSimParticle(COM_PARTICLES *com);
int SimParticle(COM_PARTICLES *com);
void AddAllParticleNoseqLink(SPR_PARTICLES *ps);


/*
  exec_level    ... 実行レベル ( GV_ACTOR_... )
  particle_name ... パーティクルの名前
  pos           ... 初期位置(ローカルもしくはワールド座標)
  v             ... 初期速度の指定(ローカルもしくはワールド座標)
  v_flag        ... 初期速度に関するフラグ
                    0 ... dataのvを使用する/ワールド、      1 ... dataのvを使用する/ローカル
					2 ... この構造体のvを使用する/ワールド、3 ... この構造体のvを使用する/ローカル
  tmat          ... ローカル -> ワールドへの変換マトリクス
                    このマトリクスは、パーティクルが存在する間、常に参照される。
  */

void *NewToolParticle_called(int exec_level,int particle_name,
							 FVECTOR *pos,FVECTOR *v,int v_flag,FMATRIX *tmat);


#endif
