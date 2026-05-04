/*
	ik_sub.h

	2000/04/06 K.Kano
	$Id: ik_sub.h,v 1.1.1.3 2002/11/19 11:43:14 Yoshizawa1 Exp $
*/


#ifndef _ik_sub_h_
#define _ik_sub_h_


void Leg_IKcalc(CONTROL *control,OBJECT *obj);
void Leg_IKcalcForRaiden(CONTROL *control,OBJECT *obj);
int Arm_IKcalc(OBJECT *obj,int objnum,OBJECT *target,int tobjnum,FVECTOR *tpoint);

void ExitArmIK(void *work);
void *InitArmIK(OBJECT *obj,int objnum,OBJECT *target,int tobjnum,FVECTOR *tpoint);
int MoveArmIK(void *work,int flag);
void ReregistTarget(void *work,OBJECT *target,int tobjnum,FVECTOR *tpoint);


#endif
