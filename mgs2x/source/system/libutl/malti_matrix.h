/*
	malti_matrix.h
		OBJを関節以外の行列を使って動かす。
		乳揺れ、腹揺れ用

	1999/12/06 K.Kano
	$Id: malti_matrix.h,v 1.1.1.3 2002/11/19 11:42:56 Yoshizawa1 Exp $
*/

#ifndef _malti_matrix_h_
#define _malti_matrix_h_


typedef struct {
    DG_VERTS_ANIME anime;
    CV2_MDL *cvd;
    int flag;

    FMATRIX *sm;
    FMATRIX *psm;
    FMATRIX *m;
    int msize;

    float *p;
    FVECTOR *v_normal;
    FVECTOR *tmp_work;
} MALTI_MATRIX_WORK;


/* 初期化 */
MALTI_MATRIX_WORK *InitMaltiMatrix(DG_OBJS *objs,CV2_DEF *def,int objnum,int msize,int flag);

/* 終了処理 */
void ExitMaltiMatrix(MALTI_MATRIX_WORK *work);

/* 通常処理 */
void MoveMaltiMatrix(MALTI_MATRIX_WORK *work);


#endif
