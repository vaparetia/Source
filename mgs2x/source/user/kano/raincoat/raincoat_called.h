/*
	raincoat_called.h
		レインコート

	2000/4/12 K.Kano
	$Id: raincoat_called.h,v 1.1.1.3 2002/11/19 11:43:33 Yoshizawa1 Exp $
*/

#ifndef _raincoat_called_h_
#define _raincoat_called_h_

void *NewRaincoat_called(OBJECT *target,int *single_names,int single_names_size,int multi_name,
			 float wind_min,float wind_max,float frame_min,float frame_max);

#endif
