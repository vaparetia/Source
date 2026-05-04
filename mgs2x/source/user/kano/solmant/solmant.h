/*
	solmant.c
		ソリダスのマント消し

	2001/05/17 K.Kano
	$Id: solmant.h,v 1.1.1.3 2002/11/19 11:43:36 Yoshizawa1 Exp $
*/


#ifndef _solmant_h_
#define _solmant_h_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

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

#ifndef PSX2
/* 共有化されたモデル頂点情報（炎エフェクト発生用） */
typedef struct {
	FVECTOR		vert ;			/* 頂点座標 */
	short		weight[4] ;		/* スケルトンへのウェイト */
	u_char		index[4] ;		/* スケルトンへのインデックス */
	short		tex_v ;			/* テクスチャＶ座標 */
	short		flag ;			/* フラグ兼カウンタ */
	FVECTOR		pos ;			/* 計算した絶対座標位置 */
} C_VERTS ;
/* 頂点共有化用 */
typedef struct _v_tag {
	struct _v_tag	*next ;
	DG_VERTEX_EVM	*vert ;
	u_char			index[4] ;		/* スケルトンへのインデックス */
} V_TAG ;
#endif


typedef struct {
	DG_EVMOBJ *evm;
	int buffer_flag;
	int buffer_size;
#ifdef PSX2
	/* PS2版ワーク */
	short *vertex_org;
	short *normal_org;
	short *uvs0_org;
	short *uvs1_org;
	short *uvs2_org;
	short *vertex[2];
	short *normal[2];
	short *uvs0[2];
	short *uvs1[2];
	short *uvs2[2];

	int stkindex;
	FVECTOR *stkvec;
	short **mdlvec;
	unsigned char *mdlweight;
#else
	/* XBOX版ワーク */
	DG_VERTEX_EVM	*verts_org ;	/* オリジナルデータ */
	DG_VERTEX_EVM	*verts[2] ;		/* アニメーション用データ */
	int				n_common_verts ;
	C_VERTS			*common_verts ;	/* 共有頂点化したエフェクト起動用頂点データ */
#endif
} EVM_VERTEXANIM;


typedef struct {
	unsigned int ptr;
	unsigned char weight[3][8];
} ANIM_POLYPTR;

typedef struct {
	unsigned int n_verts;
	ANIM_POLYPTR ptr[0];
} ANIM_DELETEPOLY;

typedef struct {
	int flags;
	int n_frames;
	ANIM_DELETEPOLY *frame_data[0];
} ANIM_DELETEPOLY_HEADER;


#endif
