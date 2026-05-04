/*
	mm_orga.h
		オルガの乳揺れ用のプログラム

	1999/12/20 K.Kano
	$Id: mm_orga.h,v 1.1.1.3 2002/11/19 11:43:15 Yoshizawa1 Exp $
*/


#ifndef _mm_orga_h_
#define _mm_orga_h_


void *InitMMOrga(DG_OBJS *objs,CV2_DEF *def);
void ExitMMOrga(void *work);
void MoveMMOrga(void *work);

void ExitEvmMMOrga(void *work);
void *_InitEvmMMOrga(DG_EVMOBJ *evm,int mune_bone,int rthigh_bone,int lthigh_bone);
void MoveEvmMMOrga(void *work);

void *NewMMOrga_called(OBJECT *object,int cv2_name);
void *NewEvmMMOrga_called(DG_EVMOBJ *evmobj);


#define N_BONE_ORGA				56

#define BONE_ORGA_MUNE			53

#define BONE_ORGA_RIGHT_THIGH	54
#define BONE_ORGA_LEFT_THIGH	55

#define InitEvmMMOrga(evm) \
	_InitEvmMMOrga(evm,BONE_ORGA_MUNE,BONE_ORGA_RIGHT_THIGH,BONE_ORGA_LEFT_THIGH)


#define N_BONE_ORGAWH			(17*2+56)

#define BONE_ORGAWH_MUNE		(17*2+53)

#define BONE_ORGAWH_RIGHT_THIGH	(17*2+54)
#define BONE_ORGAWH_LEFT_THIGH	(17*2+55)

#define InitEvmMMOrgaWithHand(evm) \
	_InitEvmMMOrga(evm,BONE_ORGAWH_MUNE,BONE_ORGAWH_RIGHT_THIGH,BONE_ORGAWH_LEFT_THIGH)


#endif
