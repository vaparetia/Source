/*
	mm_coat.h
		オルガの乳揺れ用のプログラム

	1999/12/20 K.Kano
	$Id: mm_coat.h,v 1.1.1.3 2002/11/19 11:43:32 Yoshizawa1 Exp $
*/


#ifndef _mm_coat_h_
#define _mm_coat_h_


void *InitMMCoat(DG_OBJS *objs,CV2_DEF *def);
void ExitMMCoat(void *work);
void MoveMMCoat(void *work);


#endif
