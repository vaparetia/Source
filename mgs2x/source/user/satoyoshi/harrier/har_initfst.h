/********************************************************************************/
/*	harrier.h								*/
/*	ハリアメインワーク ヘッダ						*/
/*	2001/02/23 H.Satoyoshi							*/
/*	$Id: har_initfst.h,v 1.1.1.3 2002/11/19 11:48:21 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	structure      								*/
/********************************************************************************/

#ifndef __har_initfst_h__
#define __har_initfst_h__


typedef ALIGN16_DECL(struct) {
    FVECTOR	position;
    FVECTOR	speed;
}ACRO_POINT ;


typedef ALIGN16_DECL(struct) {
    ACRO_POINT	acro;
    float	time;		// 行動開始タイミング
    int		action;		// アクションタイプ
    int		max_t;
}ACRO_POINT_EX ;

#endif





