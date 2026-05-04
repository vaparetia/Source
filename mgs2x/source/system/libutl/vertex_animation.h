/*
	vertex_animation.h
		頂点アニメーション

	1999/11/09 K.Kano
	$Id: vertex_animation.h,v 1.1.1.3 2002/11/19 11:43:01 Yoshizawa1 Exp $
*/

#ifndef _vertex_animation_h_
#define _vertex_animation_h_


typedef struct {
    DG_VERTS_ANIME anime;
    CV2_MDL **key;
    float *p;
    FVECTOR *tmp_work;
    int size;

    float *now_p;
    int count;
} VERTEX_ANIME_WORK;

VERTEX_ANIME_WORK *InitVertexAnimation(DG_OBJ *obj,CV2_MDL *mdl,int flag,int n_mdl);
void ExitVertexAnimation(VERTEX_ANIME_WORK *work);
void SimpleVertexAnimation(VERTEX_ANIME_WORK *work);

#endif
