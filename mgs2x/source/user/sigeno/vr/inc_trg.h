/*
	inc_trg.h
	ＶＲステージ子的
	2002/04/09 K.Sigeno
	$Id: inc_trg.h,v 1.1.1.3 2002/11/19 11:49:56 Yoshizawa1 Exp $
*/


extern void VR_trglifecheck(VR_TRG_Work *,GV_ACT_EX *) ;
extern void	VR_CheckMessage(VR_TRG_Work	* ) ;
extern void VR_CheckWakeEffect(VR_TRG_Work *) ;
extern void VR_rmovetrg(VR_TRG_Work * , FVECTOR *) ;
//extern void VR_checkhoming(VR_TRG_Work *) ;
extern void free_vr_trgobj(VR_TRG_Work *) ;
extern void VR_FreeMemory(VR_TRG_Work *) ;
extern void init_vr_trgobj(VR_TRG_Work * ,GV_ACT_EX *) ;
extern void VR_SetTargetPunch(VR_TRG_Work *) ;
extern void VR_SetTargetOct(VR_TRG_Work *) ;
extern void VR_SetTargetKatana(VR_TRG_Work *) ;
extern void VR_SetTargetCross(VR_TRG_Work *) ;
extern void VR_SetTargetHex(VR_TRG_Work *) ;
extern void VR_SetTargetOneTri(VR_TRG_Work *) ;
extern void VR_SetTargetTri02(VR_TRG_Work *) ;
extern void VR_AllTrgSkip(VR_TRG_Work *) ;
extern void VR_SetTargetOne(VR_TRG_Work *) ;
extern void VR_SetTargetDark(VR_TRG_Work *work) ;

extern void VR_movetrg(VR_TRG_Work * , FVECTOR *) ;

