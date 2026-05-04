/********************************************************************************/
/*	structure      								*/
/*	$Id: equip.h,v 1.1.1.3 2002/11/19 11:48:19 Yoshizawa1 Exp $									*/
/********************************************************************************/

#ifndef __equip_h__
#define __equip_h__



typedef struct {
    GV_ACT_EX		actor ;

    OBJECT		*oya_obj;
    OBJECT		body ;
    short		number;
    EQ_DATA             *eq_data ;
}Work;

#endif



