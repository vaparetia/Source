//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	trg_cmd.c
	的破壊ステージだけで使いそうなものはVR_SYSからこちらへ移動。メモリ節約のため
	2002/04/03 K.Sigeno
	$Id: trg_cmd.c,v 1.1.1.3 2002/11/19 11:49:56 Yoshizawa1 Exp $
*/

#include <stdlib.h>
#include <stdio.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>

#endif

#include	"gameheader.h"
#include	"vr.h"
#include	"libutl.h"

extern void CALL_SIG_3DPrintf(void *,FVECTOR * ,char *,int ,int ) ;

void VR_TRG_Clash_SE(FVECTOR *pos , int type){
	//破壊音 通常とＮＧの自動呼び分け
	int secode = SD_A_V_CLASHA ; /*ノーマルＳＥ*/
	if(type &VR_TARGET_TYPE_NG){
		GM_SeSetMode(SD_A_V_HITNG1,pos,GM_SEMODE_BOMB) ;
	}else {
		GM_SeSetMode(SD_A_V_HITOK1,pos,GM_SEMODE_BOMB) ;

		if(VR_COMBO_CNT >= 0) {
			if((VR_COMBO_CHAIN - 1) >= VR_COMBO_LIST_MAX){
				/*ＭＡＸコンボ*/
//printf("MAX COMBO SE!!!!!!!!\n");
				secode = SD_A_V_CLASHC ;
			}else {
				/*コンボ*/
//printf("COMBO SE!!!!!!!!\n");
				secode = SD_A_V_CLASHB ;
			}
		}else {
			/*ノーマルＳＥ*/
//printf("NORMAL SE!!!!!!!!\n");
		}
	}

	GM_SeSetMode(secode,pos,GM_SEMODE_BOMB) ;

}

int	VR_TypeFlagToNum(int flag){

	int type = VR_TRG_TYPE_TRI ;
	if(flag & (VR_TARGET_TYPE_OCT)){
		type = 	VR_TRG_TYPE_OCT ;
	}else if(flag & VR_TARGET_TYPE_KATANA){
		type = VR_TRG_TYPE_KATANA ;
	}else if(flag & VR_TARGET_TYPE_CROSS){
		type = VR_TRG_TYPE_CROSS ;
	}else if(flag & VR_TARGET_TYPE_HEX){
		type = VR_TRG_TYPE_HEX ;
	}else if(flag & VR_TARGET_TYPE_ONE){
		type = VR_TRG_TYPE_MOVE_CUBE ;
	}else if(flag & VR_TARGET_TYPE_ONE_TRI){
		type = VR_TRG_TYPE_ONE_TRI ;
	}else if(flag & VR_TARGET_TYPE_DARK_OCTA){
		type = VR_TRG_TYPE_DARK ;
	}else {
		type = VR_TRG_TYPE_TRI ;
	}

	return type ;
}


void VR_SetBlastRange(void) {
	VR_BLAST_RANGE = GCL_GetNextInt();
}

void VR_SetTrgScale(void) {
	VR_TARGET01_SCALE = ((float)GCL_GetNextInt()) / 4096.0f;
}


#define U_CHAR_MAX	(255)
void NewVRTarget1life(void){
	VR_TARGET01_LIFE_MAX = GCL_GetNextInt();
	VR_TARGET01_LIFE_MAX &= U_CHAR_MAX ;
}

void NewVRWallLife(void){
	VR_WALL_LIFE_MAX = GCL_GetNextInt();
	VR_WALL_LIFE_MAX &= U_CHAR_MAX ;
}
void NewVRCubeLife(void){
	VR_CUBE_LIFE_MAX = GCL_GetNextInt();
	VR_CUBE_LIFE_MAX &= U_CHAR_MAX ;
}

void VR_SetTargetSpeedRate(void) {
	VR_TRG_SPEED_RATE = (float)GCL_GetNextInt() /4096.0f ;
}

void VR_SetComboLimit(void) {
	VR_COMBO_LIMIT = DIRECT_TICK(GCL_GetNextInt());
}

void VR_SetComboCnt(void) {
	VR_COMBO_CNT = VR_COMBO_LIMIT ;
}
void VR_AddComboRate(int bonus) {
	VR_COMBO_RATE += bonus ;
}

void VR_RestComboCnt(void) {
	VR_COMBO_CNT = -1 ;
	VR_COMBO_RATE = 1 ;
}
/*コンボカウントを増やす前にチェックすべし*/
void VR_CheckComboChain(FVECTOR *hit) {
	FVECTOR comb_pos ;
	int		mode = 2;
//	int		mode = 3; //試しに全部点滅
	char str2[16] ;
	if(VR_COMBO_CNT >= 0 ) {

		if(VR_COMBO_CHAIN == 0){
			VR_COMBO_CHAIN = 2 ;
		}else {
			VR_COMBO_CHAIN++ ;
		}

		if(VR_COMBO_CHAIN > VR_COMBO_CHAIN_MAX){
			VR_COMBO_CHAIN_MAX = VR_COMBO_CHAIN ;
		}


/*********/
		if(VR_COMBO_LIST_MAX>0){
			if((VR_COMBO_CHAIN-2)>= VR_COMBO_LIST_MAX){
				VR_AddScore(VR_COMBO_SC_LIST[VR_COMBO_LIST_MAX-1] ) ;
				sprintf(str2,"%d COMBO!+%d\n",(VR_COMBO_CHAIN),
					VR_COMBO_SC_LIST[VR_COMBO_LIST_MAX-1]) ;
				mode = 3 ;
			}else {
				VR_AddScore(VR_COMBO_SC_LIST[VR_COMBO_CHAIN-2] ) ;
				sprintf(str2,"%d COMBO!+%d\n",(VR_COMBO_CHAIN),
					VR_COMBO_SC_LIST[VR_COMBO_CHAIN-2] ) ;
			}
		}else {
/**********/
			sprintf(str2,"%d COMBO!\n",(VR_COMBO_CHAIN)) ;
		}
		CALL_SIG_3DPrintf(VR_TRG_COMB_DISP,hit ,str2,mode,VR_COMBO_LIMIT) ;
	}
}
int VR_TrgHitFunc(void *str_work,FVECTOR	*hit,FVECTOR	*str_pos,FVECTOR *force,int mode,int type){
	FVECTOR comb_pos ;
	int	base_sc = 1;
	char str2[16] ;

#if 1
	{
		FMATRIX mat ;
		FVECTOR dir ;
		SVECTOR rot,rgb ;
		FVECTOR test ;
//		extern int NewVRSpark(FMATRIX *) ;
		extern void	NewSpark(FMATRIX *) ;
//		extern void PosBox(FVECTOR * ,float ,SVECTOR * );
		extern short GV_VecDir2X( FVECTOR * ) ;
		extern short GV_VecDir2( FVECTOR * ) ;

		if(force != NULL) {
			short s_dir ;

//printf("force x [%f]y [%f]z [%f]\n",force->vx,force->vy,force->vz);
#if 1
			dir.vx = -force->vx ;
			dir.vy = -force->vy ;
			dir.vz = -force->vz ;
			dir.vw = 1.0f ;
#else
			GV_SubVec3F( hit, &GM_PlayerPosition, &dir ) ;
#endif
			rot.vx = GV_VecDir2X( &dir ) ;
			rot.vy = GV_VecDir2( &dir ) ;
			rot.vz = 0 ;
//			DG_SetPos2( hit,&rot ) ;
			DG_SetPos2( hit,&DG_ZeroSVector ) ;

			DG_GetPos( &mat) ; 
			NewSpark(&mat) ;
//			PosBox(hit ,125.0f , NULL );
		}else {
printf("WARNING !! Spark Force is NULL!!\n");
		}
	}
#endif

	if((type == VR_TRG_TYPE_WALL)||(type == VR_TRG_TYPE_DARK)){
		return 0 ;
	}
	switch(mode){
		case VR_TRG_LEVEL1 :
			base_sc = VR_TRG_SCORE_01[type];
			break ;
		case VR_TRG_LEVEL2 :
			base_sc = VR_TRG_SCORE_02[type];
			break ;
			case VR_TRG_LEVEL3 :
			base_sc = VR_TRG_SCORE_03[type];
			break ;
		case VR_TRG_NG :
//			VR_AddScore(VR_TRG_SCORE_NG) ;
			VR_RestComboCnt() ;
			sprintf(str2,"NG!!%d\n",VR_TRG_SCORE_NG) ;
			return 0 ;
			break ;
		case VR_TRG_KATANA :
			base_sc = VR_TRG_SCORE_01[type];
			break ;
		case VR_TRG_BLAST :
			base_sc = VR_TRG_SCORE_01[type];
			break ;
	}
	{
		sprintf(str2,"%d\n",base_sc) ;
//		VR_AddScore(base_sc) ;
	}

	return base_sc ;


}

void VR_DestoryComboCheck(int bonus,FVECTOR *pos){
	VR_CheckComboChain(pos) ;
	VR_SetComboCnt() ;
//	VR_AddComboRate(bonus) ;
}
void VR_ResetCombo(FVECTOR *pos){
	FVECTOR comb_pos ;
	char str2[16] ;
	VR_COMBO_CNT = -1 ;
	VR_COMBO_CHAIN = 0 ;
	comb_pos = *pos ;
	comb_pos.vy += 500.0f ;
	comb_pos.vz -= 500.0f ;
//	sprintf(str2,"NG !!!\n") ;
	sprintf(str2,"\n") ;
	//赤で表示
	CALL_SIG_3DPrintf(VR_TRG_COMB_DISP,&comb_pos ,str2,1,DIRECT_TICK(60)) ;
}

u_char VR_GetPartsDmg(int type,int level) {
	u_char dmg = 0;
	ASSERT(type<VR_TRG_TYPE_MAX) ;
	switch(level){
		case 1 :
			dmg = VR_TRG_STRENGTH_01[type];
			break;
		case 2 :
			dmg = VR_TRG_STRENGTH_02[type];
			break;
		case 3 :
			dmg = VR_TRG_STRENGTH_03[type];
			break;
		default :
			ASSERT(0) ;
	}
	return dmg ;
}

void VR_SetTargetScore(void) {
	int type ;
	type = GCL_GetNextInt() ;
	ASSERT(type<VR_TRG_TYPE_MAX) ;

	if( GCL_NextStr() != NULL ){
		VR_TRG_SCORE_01[type] = GCL_GetNextInt() ;
printf("VR_TRG_SCORE_01[type][%d]\n",VR_TRG_SCORE_01[type]);
	}
	if( GCL_NextStr() != NULL ){
		VR_TRG_SCORE_02[type] = GCL_GetNextInt() ;
printf("VR_TRG_SCORE_02[type][%d]\n",VR_TRG_SCORE_02[type]);
	}
	if( GCL_NextStr() != NULL ){
		VR_TRG_SCORE_03[type] = GCL_GetNextInt() ;
printf("VR_TRG_SCORE_03[type][%d]\n",VR_TRG_SCORE_03[type]);
	}
}
void VR_SetTargetNGScore(void) {
	int type ;
	VR_TRG_SCORE_NG = GCL_GetNextInt() ;
}
void VR_SetTargetStrength(void) {
	int type ;
	type = GCL_GetNextInt() ;
	ASSERT(type<VR_TRG_TYPE_MAX) ;

	if( GCL_NextStr() != NULL ){
		VR_TRG_STRENGTH_01[type] = GCL_GetNextInt() ;
	}
	if( GCL_NextStr() != NULL ){
		VR_TRG_STRENGTH_02[type] = GCL_GetNextInt() ;
	}
	if( GCL_NextStr() != NULL ){
		VR_TRG_STRENGTH_03[type] = GCL_GetNextInt() ;
	}
}

#define VEL_RATE (2.0f)
#define VEL1 (50.0f*VEL_RATE)
#define VEL2 (55.0f*VEL_RATE)
#define VEL3 (60.0f*VEL_RATE)
#define ROT1 (50.0f*1.0f)
#define ROT2 (55.0f*1.0f)
#define ROT3 (60.0f*1.0f)
#define	BREAK_DECAY_TIME	(3)
#define VR_FADE_RATE (0.05f)
#define ROT (63)

void CallBreakObj(int ef_mdl_code ,FMATRIX *world){
//	extern NewSIG_VR_Noise((FVECTOR *)&work->body.objs->world.m[3] ,0,120)	;
extern void NewSIG_VR_Noise(FVECTOR * ,int ,int) ;
extern void *NewSigBreakObj(int mdl,FMATRIX *world,float speed ,float decay,CVECTOR *col,SVECTOR	*rot,int decay_time) ;

	CVECTOR col ;
	SVECTOR	rot ;

	short rot_p ;

	rot_p = ((irnd()>>8)%ROT) - ROT/2 ;

	rot.vx = rot_p ;
	rot_p = ((irnd()>>8)%ROT) - ROT/2 ;
	rot.vy = rot_p ;
	rot_p = ((irnd()>>8)%ROT) - ROT/2 ;
	rot.vz = rot_p ;

	col.r = 108 ;col.g = 0; col.b = 50 ;
//	NewSigBreakObj(ef_mdl_code ,world,VEL1,VR_FADE_RATE,&col ,&rot,BREAK_DECAY_TIME) ;

	col.r = 0 ;col.g = 108; col.b = 108 ;
	rot.vy = (float)rot.vy * 1.2f ;
	NewSigBreakObj(ef_mdl_code ,world,VEL2,VR_FADE_RATE, &col,&rot,BREAK_DECAY_TIME) ;

	col.r = 0 ;col.g = 36; col.b = 108 ;
	rot.vy = (float)rot.vy * 1.2f ;
	NewSigBreakObj(ef_mdl_code ,world,VEL3,VR_FADE_RATE, &col,&rot,BREAK_DECAY_TIME) ;

	NewSIG_VR_Noise((FVECTOR *)&world->m[3] ,0,120)	;
}
void VR_SetTargetHideCycle(void) {
	VR_TARGET_HIDE_CYCLE = VR_TARGET_APPEAR_CYCLE = DIRECT_TICK(GCL_GetNextInt());
}
void VR_SetTargetHideAppearCycle(void) {
	VR_TARGET_APPEAR_CYCLE = DIRECT_TICK(GCL_GetNextInt());
	VR_TARGET_HIDE_CYCLE = DIRECT_TICK(GCL_GetNextInt());
}
