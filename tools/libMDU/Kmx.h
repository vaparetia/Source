/*
   Kmx.h

   Kmx,Kms変換メイン

   by K.Kano 3/1/2000
   $Id: Kmx.h,v 1.13 2002/06/07 13:01:29 usr01363 Exp $

   Konami Computer Entertainment Japan West   
*/

#ifndef _Kmx_h_
#define _Kmx_h_


/* Mdl2Kmx.c */
KMX_DEF *MDU_Mdl2Kmx(LPCSTR name,LPP3DXYZ scale,HP3DMODEL hP3DModel,int skel);
void CountMesh( HP3DMODEL  hP3DModel, HP3DOBJECT hP3DObject, int *skel, int *mesh );

/* DivideKmx.c */
KMX_DEF *MDU_DivideKmx(KMX_DEF *def,SVECTOR *div,int vert);
KMX_DEF *MDU_SplitKmx(KMX_DEF *def);

/* CheckPrims.c */
void Kmx_CheckNormal(KMX_DEF *def);
int Kmx_CheckTexture(KMX_DEF *def,int check_flag);

/* Kmx2Kms.c */
KMX_DEF *KmxDividePrimTriangle(KMX_DEF *olddef);
int MDU_Kmx2Kms2(KMX_DEF *kmxdef,KMS2_DEF **def,CV2_OBJS **cdef,int fast_flag,
				 int multitex_flag,int ovarlap_flag, int etc_flag );

/* Kmx2Kmsx.c */
int MDU_Kmx2Kmsx(KMX_DEF *kmxdef,KMS2_DEF **def,CV2_OBJS **cdef,int fast_flag,
				 int multitex_flag,int overlap_flag, int etc_flag, int multiuv_flag );

/* Overlap.c */
void Kmx_RemoveOverlap(KMX_DEF *kmx,int ovarlap_flag);


#endif
