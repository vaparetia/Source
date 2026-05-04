/*
	makequat.c
		しょっちゅう使うので独立させた

	1999/11/4 K.Kano
	$Id: makequat.x,v 1.1.1.3 2002/11/19 11:43:14 Yoshizawa1 Exp $
*/

#ifndef _makequat_h_
#define _makequat_h_

int UTL_MakeQuat(FVECTOR *q,FVECTOR *x0,FVECTOR *x1,FVECTOR *srcv);
int UTL_MakeQuatM(FVECTOR *q,FVECTOR *x0,FVECTOR *x1,FMATRIX *m,FVECTOR *srcv);

#endif
