/*
	resman.c
		ローカルリソース管理

	2001/06/12 K.Kano
	$Id: resman.h,v 1.1.1.3 2002/11/19 11:43:34 Yoshizawa1 Exp $
*/


#ifndef _resman_h_
#define _resman_h_


/* 常駐でないリソースの管理 */
void *GetLocalResource( int ref_id, int offset );


#endif
