//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	itembreak.c
	装備品モデルを分解飛散
	2002/04/30 K.Sigeno
	$Id: itembreak.c,v 1.1.1.3 2002/11/19 11:49:47 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>

#include <sifdev.h>
#include <libvu0.h>

#endif


#include "gameheader.h"
#include "libutl.h"

#include "korekado/enemy/enemy.h"



#define NORMAL_MODE		(0x00000000)
#define AMB_MODE		(0x00000001)
#define BUILD_Y			(0x00000002)
#define BUILD_UP		(0x00000004)
#define NO_CHECK_SW		(0x00000008)
#define DISAPPEAR_MODE		(0x00000010)
#define BODY_MODE			(0x00000020)
#define BUILD_UP_Z			(0x00000040)
#define RAND_VERTS			(0x00000080)
#define MOT_TR			(0x00000100)


#define ITEM_BREAK (RAND_VERTS|DISAPPEAR_MODE|NO_CHECK_SW)
#define ITEM_APPEAR (RAND_VERTS|NO_CHECK_SW)

//#define ITEM_BREAK (BUILD_UP_Z|DISAPPEAR_MODE|NO_CHECK_SW)
//#define ITEM_BREAK (BUILD_UP|DISAPPEAR_MODE|NO_CHECK_SW)


#define BREAK_LEN (75.0f)
#define TIME_A	(90)
#define SLING_TIME_A (30)
#define TIME_B	(0)
#define SLING_DELAY (60)

extern void *NewSigBreakObj3(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,
	int delay,int *sw,FVECTOR *shift,SVECTOR *rot) ;

//#define ENE_TYPE_WATCHER		0x00000001
//#define ENE_TYPE_ATTACKER		0x00000002


/* AK:タンカーゴル兵スリング */
static FVECTOR	AK_SHIFT = { -200.0, 110.0, 200.0 } ;
static SVECTOR	AK_ROT = { -338, 790, 130 } ;	/* Z->X->Y */

/* ABAKAN:プラントゴル兵スリング */
static FVECTOR	ABAKAN_SHIFT = { -200.0, 110.0, 200.0 } ;
static SVECTOR	ABAKAN_ROT = { -338, 790, 130 } ;	/* Z->X->Y */

/* M4:チョーカースリング */
static FVECTOR	M4_SHIFT = { -100.0, 0.0, 250.0 } ;
static SVECTOR	M4_ROT = { -258, 890, 100 } ;	/* Z->X->Y */

/*RADIO*/
static FVECTOR	RAD_W_Shift = { -120.0, 25.0, -97.5 } ;
static SVECTOR	RAD_W_Rot = { 4028, 3777, 2048 } ;


static FVECTOR	KATANA_SAYA_Shift = { 164.0f, 40.0f, 6.0f } ;
static SVECTOR	KATANA_SAYA_Rot = { 0, 0, 0 } ;

static FVECTOR	KATANA_STRAP_Shift = { 0.0f, 0.0f, 0.0f } ;
static SVECTOR	KATANA_STRAP_Rot = { 0, 0, 0 } ;


static FVECTOR shift[]={
    { -185.0f,  27.5f,  61.5f,   0.0f, },
    {  -67.5f,  -2.5f, 152.5f,   0.0f, },
    {  120.0f,  22.5f, -60.0f,   0.0f, },
    {  140.0f,  37.0f, -49.0f,   0.0f, },
};
static FVECTOR shift2[]={
    {  -65.0f, -10.0f,  42.5f,   0.0f, },
    {   32.5f,-140.0f, 112.5f,   0.0f, },
    {   20.0f, -92.5f,-117.5f,   0.0f, },
};

static FVECTOR bp_shift ={
	0.0f, -40.0f,-140.0f,   0.0f
} ;

static FVECTOR tng_mag_shift= {
    150.0f, 40.0f, 37.0f, 0.0f
};

static FVECTOR sling_shift= {
    0.0f, 0.0f, -10.0f, 0.0f
};


static const SVECTOR rot1[]={
    {  -68, 0, 0, 0, },
} ;

#define BACK_SLING_MDL (13754284) //スリング背中側
#define GBS_HLST	(2691136)	/* GV_StrCode("gbs_hlst") */ 
#define GBS_MAG		(6380306)	/* GV_StrCode("gbs_mag") */ 
#define GBS_KNIF	(2791154)	/* GV_StrCode("gbs_knif") */ 
#define GPS_KNIF	(5459626)	/* GV_StrCode("gps_knife") */ 
#define GBS_BP		(5966212)	/* GV_StrCode("gbs_bp") */ 
#define TNG_MAG		(9531134)	/* GV_StrCode("tng_magazine") */ 
#define ENE_RAD		(119940)	/*GV_StrCode("rad")*/

/*
#define HUMAN21_MIGI_ASHI1       (13)
#define HUMAN21_MIGI_ASHI2       (14)
#define HUMAN21_MIGI_KAKATO      (15)
#define HUMAN21_MIGI_TSUMASAKI   (16)
#define HUMAN21_HIDARI_ASHI1     (17)
*/


/*天狗Ｐ９０と茶警備 都市警備 ＡＫ ＡＢＫに対応*/
static void SIG_EneEquipBreak2(DG_OBJS *objs ,int mode){
extern void *NewSigBreakObj5(DG_OBJS *objs,int mode ,float len,int time_a ,int delay,int *sw) ;
	if(mode == 0){
		NewSigBreakObj5(objs,ITEM_APPEAR, 0.0f,8 ,7,(int *)NULL) ;
	}else {
		NewSigBreakObj5(objs,ITEM_BREAK, 0.0f,8 ,7,(int *)NULL) ;
	}
}
void SIG_EneEquipBreak(OBJECT	*body ,int type){
	int time_a = TIME_A, time_b = TIME_B ;
	int sling_time_a = SLING_TIME_A ,sling_delay = SLING_DELAY ;

	if ( GM_VRStatus & GM_VR_CLEAR ) {
		/*クリア時は高速消え*/
		time_a /= 2 ;
		time_b /= 2 ;
		sling_time_a /= 2 ;
		sling_delay /= 2 ;
	}


	if(type & ENE_TYPE_TNG_A) {

		if(type & ENE_TYPE_KATANA){
			//KATANA鞘とストラップ
			NewSigBreakObj3(GV_StrCode("tng_saya"),&(body->objs->objs[HUMAN21_KOSHI].world),ITEM_BREAK ,
			BREAK_LEN,time_a ,time_b,(int *)NULL,&KATANA_SAYA_Shift, &KATANA_SAYA_Rot) ;
			NewSigBreakObj3(GV_StrCode("tng_strap"),&(body->objs->objs[HUMAN21_KOSHI].world),ITEM_BREAK ,
			BREAK_LEN,time_a ,time_b,(int *)NULL,&KATANA_STRAP_Shift, &KATANA_STRAP_Rot) ;
		}else {
			//P90とマガジン 
			NewSigBreakObj3(E_WP_P90,&(body->objs->objs[HUMAN21_MUNE].world),ITEM_BREAK ,
			BREAK_LEN,time_a ,time_b,(int *)NULL,&AK_SHIFT, &AK_ROT) ;
			NewSigBreakObj3(TNG_MAG,&(body->objs->objs[HUMAN21_KOSHI].world),ITEM_BREAK ,
			BREAK_LEN,time_a ,time_b,(int *)NULL,&tng_mag_shift, &DG_ZeroSVector) ;
		}
	}else if(type & ENE_TYPE_WATCHER){
		//背中スリング
		if(!(type & ENE_TYPE_VR)){
			NewSigBreakObj3(BACK_SLING_MDL,&(body->objs->objs[HUMAN21_MUNE].world),ITEM_BREAK ,
			BREAK_LEN,sling_time_a ,sling_delay,(int *)NULL,&sling_shift, &DG_ZeroSVector) ;
			//ホルスター
			NewSigBreakObj3(GBS_HLST,&(body->objs->objs[HUMAN21_KOSHI].world),ITEM_BREAK ,
				BREAK_LEN,time_a ,time_b,(int *)NULL,shift+0, &DG_ZeroSVector) ;
		}
		//無線機
		NewSigBreakObj3(ENE_RAD,&(body->objs->objs[HUMAN21_KOSHI].world),ITEM_BREAK ,
		BREAK_LEN,time_a ,time_b,(int *)NULL,&RAD_W_Shift, &RAD_W_Rot) ;

		if(type & ENE_TYPE_ABAKAN) {
			//アバカン
			NewSigBreakObj3(E_WP_ABAKAN,&(body->objs->objs[HUMAN21_MUNE].world),ITEM_BREAK ,
			BREAK_LEN,time_a ,time_b,(int *)NULL,&ABAKAN_SHIFT, &ABAKAN_ROT) ;

			if(!(type & ENE_TYPE_VR)){
				NewSigBreakObj3(GBS_MAG,&(body->objs->objs[HUMAN21_KOSHI].world),ITEM_BREAK ,
				BREAK_LEN,time_a ,time_b,(int *)NULL,shift+3, &DG_ZeroSVector) ;
	
				NewSigBreakObj3(GPS_KNIF,&(body->objs->objs[HUMAN21_KOSHI].world),ITEM_BREAK ,
				BREAK_LEN,time_a ,time_b,(int *)NULL,shift+1, &DG_ZeroSVector) ;
			}
		}else {
			//AK
//HUMAN21_MIGI_TE
			NewSigBreakObj3(E_WP_AKS_SP,&(body->objs->objs[HUMAN21_MUNE].world),ITEM_BREAK ,
			BREAK_LEN,time_a ,time_b,(int *)NULL,&AK_SHIFT, &AK_ROT) ;

			if(!(type & ENE_TYPE_VR)){
				NewSigBreakObj3(GBS_MAG,&(body->objs->objs[HUMAN21_KOSHI].world),ITEM_BREAK ,
				BREAK_LEN,time_a ,time_b,(int *)NULL,shift+2, &DG_ZeroSVector) ;
				NewSigBreakObj3(GBS_KNIF,&(body->objs->objs[HUMAN21_KOSHI].world),ITEM_BREAK ,
				BREAK_LEN,time_a ,time_b,(int *)NULL,shift+1, &DG_ZeroSVector) ;
			}
		}
		/*リュック*/
		if ( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
			NewSigBreakObj3(GBS_BP,&(body->objs->objs[HUMAN21_MUNE].world),ITEM_BREAK ,
			BREAK_LEN,time_a ,time_b,(int *)NULL,&bp_shift, &DG_ZeroSVector) ;
		}else {
		}
	}
}

void *EnemyAppearEf(OBJECT *body,OBJECT *weapon,int type){
	extern void *NewENE_BreakBody(OBJECT *,int , void *) ;
	switch (type){
		case 0 :
			SIG_EneEquipBreak2(weapon->objs,0) ;
			return NewENE_BreakBody(body,9,NULL) ;
			break;
		case 1 :
			/*消失*/
			SIG_EneEquipBreak2(weapon->objs,1) ;
			return NewENE_BreakBody(body,10,NULL) ;
			break ;
		case 2 :
			/*消失*/
			return NewENE_BreakBody(body,11,NULL) ;
			break ;
	}
	return NULL ;
}
