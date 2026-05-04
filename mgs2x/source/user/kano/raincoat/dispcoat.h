/*
	dispcoat.h
		オルガの乳揺れ用のプログラム

	2000/04/05 K.Kano
	$Id: dispcoat.h,v 1.1.1.3 2002/11/19 11:43:32 Yoshizawa1 Exp $
*/


#ifndef _dispcoat_h_
#define _dispcoat_h_


typedef struct {
    OBJECT   *target;

    DG_OBJS   *objs;
    DG_EVMOBJ *evm;
} DISP_COAT;

void ExitDispCoat(DISP_COAT *work);
DISP_COAT *InitDispCoat(int coat_kms,int coat_evm,OBJECT *target);
void MoveDispCoat(DISP_COAT *work);


#endif
