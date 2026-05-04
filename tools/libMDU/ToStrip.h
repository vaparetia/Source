/*
   ToStrip.h

   共有頂点形式の中間フォーマットを、ストリップに直すためのサブルーチン

   by K.Kano , 3/1/2000

   $Id: ToStrip.h,v 1.10 2002/08/26 11:22:09 usr01363 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#ifndef _ToStrip_h_
#define _ToStrip_h_


#define MAX_OBJIDS	8
#define MAX_TIDS	8

#define MAX_VERTEX_WEIGHTS		4


typedef struct _PRIM_POINT {
    u_int vid;
    u_int nid;
    TVECTOR *uvs;
    int draw_flag;

	KMX_PRIMS *prim;
	int v_order;
} PRIM_POINT;

typedef struct _PRIM {
    struct _PRIM *prea,*next;
    int n_tid;
    u_int *tid;
    PRIM_POINT p[3];

	/* ストリップ作成の際に、このプリミティブは既に出力済かどうかを
	   判定するフラグ (ToStrip.c : Set_Strip_Algo_1) */
	/* ストリップ作成の前処理として、プリミティブのグループ分けを行なうが、
	   プリミティブがどのグループに所属しているかのグループ番号を格納
	   (DivPrims.c : Kmx_Divide_By_Line) */
    int flag;
} PRIM;

typedef struct _DIV_PRIM {
    int n_prims;
    int n_tid;
    u_int *tid;
    u_int have_env;
    PRIM prims;
} DIV_PRIM;

typedef struct _DIV_PRIMS {
    int n_div_prims;
    int start_single;
    DIV_PRIM div_prims[NULL_ARRAY];
} DIV_PRIMS;

typedef struct _STRIP {
    int n_points;
    int n_tid;
    u_int *tid;
    u_int have_env;
    int n_objids;
	int max_objids;
    int objids[MAX_OBJIDS];
    PRIM_POINT p[NULL_ARRAY];
} STRIP;


/* DivPrims.c */
DIV_PRIMS *Kmx2DivPrims(KMX_MDL *mdl);
DIV_PRIMS *Evf2DivPrims(EVF_MESH *mesh,int n_x_models);


/* ToStrip.c */
PRIM *Prim_Alloc(int n_tid);
int Div_Prim_Alloc(DIV_PRIM *p,int n_tid);
DIV_PRIMS *Div_Prims_Alloc(int n_div_prims);

void Prim_Free(PRIM *p);
void Div_Prim_Free(DIV_PRIM *p);
void Div_Prims_Free(DIV_PRIMS *div_prims);

void Div_Prims_Move(DIV_PRIMS *r,DIV_PRIMS *a);


int Strip_Uvs_Alloc(STRIP *s,int index);
void Strip_Free(STRIP *strip);
STRIP *Strip_Alloc(int n_tid,int strip_len);
void Strips_Free(STRIP **strips,int n_strips);
STRIP **Strips_Alloc(int n_strips);
int Cmp_Prim_Point(PRIM_POINT *p1,PRIM_POINT *p2,int n_tid);

STRIP **EasyStrip(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points);
STRIP **Strip_Algo_1(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points);
int Union_STRIP(STRIP **strips,int n_strips,int n_max_points);

STRIP **Strip_Algo_1_X(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points);
int Union_STRIP_X(STRIP **strips,int n_strips,int n_max_points);
void SortStrip(TEXINFO *texs,STRIP **strips,int n_strips);


#endif
