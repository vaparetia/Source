/*

  コンフ用　ビルトイン関数

  2000/08/23 T.Morita
  $Id: demosce.h,v 1.1.1.3 2002/11/19 11:45:11 Yoshizawa1 Exp $

 */
void  sceVu0CopyVector( FVECTOR *v0,  FVECTOR *v1);
void  sceVu0CopyVectorXYZ( FVECTOR *v0,  FVECTOR *v1);
void  sceVu0FTOI0Vector( IVECTOR *v0,  FVECTOR *v1);
void  sceVu0FTOI4Vector( IVECTOR *v0,  FVECTOR *v1);
void  sceVu0ITOF0Vector( FVECTOR *v0,  IVECTOR *v1);
void  sceVu0ITOF4Vector( FVECTOR *v0,  IVECTOR *v1);
void  sceVu0ScaleVector( FVECTOR *v0,  FVECTOR *v1, float s);
void  sceVu0ScaleVectorXYZ( FVECTOR *v0,  FVECTOR *v1, float s);
void  sceVu0AddVector( FVECTOR *v0,  FVECTOR *v1,  FVECTOR *v2);
void  sceVu0SubVector( FVECTOR *v0,  FVECTOR *v1,  FVECTOR *v2);
void  sceVu0MulVector( FVECTOR *v0,  FVECTOR *v1,  FVECTOR *v2);
void  sceVu0InterVector( FVECTOR *v0,  FVECTOR *v1,  FVECTOR *v2, float r);
void  sceVu0InterVectorXYZ( FVECTOR *v0,  FVECTOR *v1,  FVECTOR *v2, float r);
void  sceVu0DivVector( FVECTOR *v0,  FVECTOR *v1, float q);
void  sceVu0DivVectorXYZ( FVECTOR *v0,  FVECTOR *v1, float q);
float sceVu0InnerProduct( FVECTOR *v0,  FVECTOR *v1);
void  sceVu0OuterProduct( FVECTOR *v0,  FVECTOR *v1,  FVECTOR *v2);
void  sceVu0Normalize( FVECTOR *v0,  FVECTOR *v1);
void  sceVu0ApplyMatrix( FVECTOR *v0,  FMATRIX *m,  FVECTOR *v1);
void  sceVu0UnitMatrix( FMATRIX *m);
void  sceVu0CopyMatrix( FMATRIX *m0,  FMATRIX *m1);
void  sceVu0TransposeMatrix( FMATRIX *m0,  FMATRIX *m1);
void  sceVu0MulMatrix( FMATRIX *m0,  FMATRIX *m1,  FMATRIX *m2);
void  sceVu0InversMatrix( FMATRIX *m0,  FMATRIX *m1);
void  sceVu0RotMatrixX( FMATRIX *m0,  FMATRIX *m1, float rx);
void  sceVu0RotMatrixY( FMATRIX *m0,  FMATRIX *m1, float ry);
void  sceVu0RotMatrixZ( FMATRIX *m0,  FMATRIX *m1, float rz);
void  sceVu0RotMatrix( FMATRIX *m0,  FMATRIX *m1,  FVECTOR *rot);
void  sceVu0TransMatrix( FMATRIX *m0,  FMATRIX *m1,  FVECTOR *tv);
