/*
	cergei_eri_called.h
		マルチウェイトのセルゲイの襟揺らしその他

	2001/03/22 K.Kano
	$Id: cergei_eri_called.h,v 1.1.1.3 2002/11/19 11:43:06 Yoshizawa1 Exp $
*/


#ifndef _cergei_eri_called_h_
#define _cergei_eri_called_h_


void *NewCergeiEri_called(OBJECT *target,
						  int n_targets,int *target_nums,FVECTOR *imgpnt,
						  int deg_limit,float pa,float k,float a);
void *NewCergeiEri_demo(int sample_num,OBJECT *object);


#endif
