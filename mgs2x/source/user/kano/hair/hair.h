/*
	hair.h
		髪の毛(長髪タイプ)シミュレーション

	2000/06/02 K.Kano
	$Id: hair.h,v 1.1.1.3 2002/11/19 11:43:11 Yoshizawa1 Exp $
*/


#ifndef _hair_h_
#define _hair_h_

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>


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

#include "gameheader.h"
#include "libutl.h"
#include "utl_dma.h"


#define N_OBJ_MATRIX	16


typedef struct {

	/* モデルデータ */
	DG_OBJS *kms;
	DG_EVMOBJ *evm;

	FMATRIX light[2];

	FVECTOR target_pos;

    /* モデルに取り付ける場合に用いるワールドマトリクス */
    FMATRIX root;
    FMATRIX *tmat[N_OBJ_MATRIX];

    /* モデルにつける場合の、相手のOBJECTとOBJ番号 */
    OBJECT *target;
    int tobjnum[N_OBJ_MATRIX];
	int n_parents;

	int n_models;
	int n_ex_models;

    /* 前回のワールド座標 */
	FVECTOR *x;

    /* 前回の速度 */
	FVECTOR *v;

    /* 基準張力 */
	FVECTOR *base_t;

	/* 前回の回転、配列個数はn_ex_models */
	FVECTOR *lrots;

	/* 前回のワールドマトリクス、配列個数はn_ex_models */
	FMATRIX *wmat;

	/* 親から自分の原点までのオフセット、配列個数はn_ex_models */
	FVECTOR *t;

	/* 親のスケルトン番号、配列個数はn_ex_models */
	int *parent;

	/* シミュレート動作を行わないスケルトンの番号。
	   親のスケルトンを辿って計算する。*/
	int *b_parent;


    /* オブジェごとの質量の配列 */
	float m;

    /* 空気によって加えられる力の割合の配列 */
	float pa;

    /* オブジェごとの質量の逆数の配列 */
    float inv_m;

    /* 速度に対する抵抗値 */
    float k;

    /* 壁に対する反発係数 */
    float we;

    /* 壁検出距離 */
    int wl;

	/* 髪の毛の曲がり具合を戻そうとする力のパラメータ */
	float deg_param;

    /* 楕円球用パラメータ */
	float param_oval;

	/* 全体の移動の何割を無視するかのパラメータ */
	float mov_rate,ymov_rate;

    /* 当たり判定を行なうかどうかのフラグ */
    int collision_flag;

	/* あるモデルについていく場合、
	   そのモデルのどのパーツと当たり判定を行うかの番号に配列
	   collision_flagにはその数がはいる。*/
	unsigned char collision_objs[32];

    /* 強制的に表示するかどうかのフラグ */
    int visible_flag;
	int mesg_disp;

	/* ライトの影響をアンビエント主体に変更する */
	int light_flag;

    /* バウンディングのチェックに使うモデル */
    DG_DEF *boundmodel;

	/* 非表示に用いるフラグ */
	int invisible_flags;

	/* 計算を行うかどうかのフラグ */
	int calc_flag;

	/* 髪の毛を逆立てるかどうかのフラグ */
	float calc_angree_param;
	int calc_angree_frame;
	int calc_angree_fade_frame;

	/* ステージ置きの際のターゲット */
	TARGET *targetsys;

#ifdef DEBUG_MODE
	int debug_flag;
#endif

} HAIR_WORK;


typedef struct {

    /* 空気によって加えられる力の割合の配列 */
    float pa;

    /* オブジェごとの質量の配列 */
    float m;

    /* 速度に対する抵抗値 */
    float k;

	/* 髪の毛の曲がり具合を戻そうとする力のパラメータ */
	float deg_param;

    /* 壁に対する反発係数 */
    float we;

    /* 壁検出距離 */
    int wl;

	/* 全体の移動の何割を無視するかのパラメータ */
	float mov_rate;
	float ymov_rate;

} SAMPLE_HAIR_PARAMETER;


#define HAIR_TAIL_LENGTH		50.0f


#define HAIR_SIGNAL_CALC_FLAG	0x4001


//オセロットの当たりチェック関節の配列
extern const u_char Demo_CollisionObjs_Rev[3];


void CalcHairCoordinate(HAIR_WORK *work);
void hair_target_callback(TARGET *ofs,TARGET *def,void *param);

extern const unsigned char *hair_colcheck[];
extern const unsigned char n_hair_colcheck[];
extern const unsigned char *hair_root[];
extern const unsigned char n_hair_root[];


void HairEvmCalcFirst(HAIR_WORK *work);
int InitHairEvm(HAIR_WORK *work,int model_name,int draw_flag);
void ExitHairEvm(HAIR_WORK *work);
void MoveHairEvm(HAIR_WORK *work,HZX_GROUP_ID hzx_id);


#endif
