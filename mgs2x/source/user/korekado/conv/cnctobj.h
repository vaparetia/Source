/*
	cnctobj.h
	コネクトオブジェ

	2001/02/07 Y.Korekado
	$Id: cnctobj.h,v 1.1.1.3 2002/11/19 11:44:02 Yoshizawa1 Exp $
*/

#ifndef __CNCTOBJ____
#define __CNCTOBJ____

typedef	struct	{
	OBJECT		body ;		/* モデル */
	OBJECT		*cnct_obj ;	/* 親モデル */

	FVECTOR		shift ;
	SVECTOR		rot ;

	int			cnct_num ;
} CNCTOBJ ;

/*----------------------------------------------------------------*/
void CNCT_CnctObjNum( CNCTOBJ *cnct, int num ) ;
void CNCT_CnctObjShift( CNCTOBJ *cnct, FVECTOR *shift ) ;
void CNCT_CnctObjRot( CNCTOBJ *cnct, SVECTOR *rot ) ;
void CNCT_CnctObjObject( CNCTOBJ *cnct, OBJECT *obj ) ;
void *NewConnectObject2( CNCTOBJ *cnct, OBJECT *cnct_obj, int cnct_num, FVECTOR *shift, SVECTOR *rot, int model ) ;
#endif
