/*
	libutl.x
	各種ユーティリティライブラリプロトタイプ宣言

	1999/08/16 K.Takabe
	$Id: libutl.x,v 1.1.1.3 2002/11/19 11:42:55 Yoshizawa1 Exp $

*/

/* util_dmy.c */


/* part_sub.c */

/* Actから呼び出す、パーティクル汎用制御ルーチン */
void ParticleSub(CONTROL_PARTICLES *cparts);

/* パーティクル汎用制御ルーチンが使用するワークを初期化。
   必要な値をセットする必要があるが、メモリの確保を自動的に行なってくれる。*/
int InitParticle(CONTROL_PARTICLES *cparts);


/* vertex_search.c */
void VertexSearch(FVECTOR *vans,FVECTOR *nans,DG_OBJS *objs,int objnum,FVECTOR *target);

/* vectomat.c */
void  BIG_VectoMat( FMATRIX *world, FVECTOR *force, FVECTOR *hit, int mode ) ;
void  UTL_VectoMat( FMATRIX *world, FVECTOR *force, FVECTOR *hit, int axis ) ;
