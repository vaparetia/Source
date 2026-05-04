/*
   Evf.h

   Evf,Evm変換メイン

   by K.Kano 3/7/2000
   $Id: Evf.h,v 1.5 2002/03/28 04:38:18 usr03700 Exp $

   Konami Computer Entertainment Japan West   
*/

#ifndef _Evf_h_
#define _Evf_h_


/* CheckPrims.c */
int Evf_CheckTexture(EVF_DEF *def,int check_flag);

/* Mdl2Evf.c */
EVF_DEF	*MDU_Mdl2Evf(LPCSTR name,LPP3DXYZ scale,HP3DMODEL hP3DModel);

/* Evf2Evm.c */
EVM_DEF *MDU_Evf2Evm(EVF_DEF *evfdef,int fast_flag,int fix_flag, int etc_flag );

/* Overlap.c */
void Evf_RemoveOverlap(EVF_DEF *evf);


#endif
