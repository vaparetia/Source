/*
	va_coat.h
		レインコートのコートの揺れ

	2000/04/05 K.Kano
	$Id: va_coat.h,v 1.1.1.3 2002/11/19 11:43:33 Yoshizawa1 Exp $
*/


#ifndef _va_coat_h_
#define _va_coat_h_


void ExitVACoat(void *work);
void *InitVACoat(DG_OBJS *objs,CV2_DEF **def,int cv2def_size,
		 float wind_min,float wind_max,float speed_min,float speed_max);
void MoveVACoat(void *work);


#endif
