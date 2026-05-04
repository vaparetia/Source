/*

	premove.c
	デザイナープレビュー環境用：物体移動
	2000/02/11 K.Kano
	$Id: premove.h,v 1.1.1.3 2002/11/19 11:43:30 Yoshizawa1 Exp $

*/


#ifndef _premove_h_
#define _premove_h_

int PreviewVMoveXY(FVECTOR *pos,float v);
int PreviewVMoveZ(FVECTOR *pos,float v);
int PreviewVMoveXYZ(FVECTOR *pos,float v);
int PreviewMoveX(FVECTOR *pos,float v);
int PreviewMoveY(FVECTOR *pos,float v);
int PreviewMoveZ(FVECTOR *pos,float v);

int PreviewVRotXY(FMATRIX *world,SVECTOR *rot,int v);
int PreviewVRotZ(FMATRIX *world,SVECTOR *rot,int v);
int PreviewVRotXYZ(FMATRIX *world,SVECTOR *rot,int v);
int PreviewRotX(SVECTOR *rot,int v);
int PreviewRotY(SVECTOR *rot,int v);
int PreviewRotZ(SVECTOR *rot,int v);

#endif
