/********************************************************************************/
/*	har_pilots.h								*/
/*	ハリアパイロットワーク							*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_pirots.h,v 1.1.1.3 2002/11/19 11:48:24 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	structure      								*/
/********************************************************************************/

#ifndef __har_harpil_h__
#define __har_harpil_h__

typedef struct {
    GV_ACT_EX		actor ;
    CONTROL		*oya_control;
    OBJECT		body ;
    OBJECT		body2 ;
    OBJECT		canp ;
    OBJECT		canp_br ;
    OBJECT		frame ;
    SVECTOR		rots[3];
}Pil_Work;
#endif

