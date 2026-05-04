//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	inc_trg.c
	ＶＲステージ子的
	2002/04/09 K.Sigeno
	$Id: inc_trg.c,v 1.1.1.3 2002/11/19 11:49:55 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include "korekado/enemy/enemy.h"
#include "korekado/enemy/enemy.x"

#include "vr.h"
#include "trg_cmd.h"

#include "BP_Misc.h"

extern	void	*NewBlast3( FVECTOR *, int, int, int, int, int, int, int ) ;
extern	void *NewHomingSight( int *,FMATRIX *,float,float,float ) ;

//ポイント時
static CVECTOR	VR_HLT_DEF	= {5,48,32};
static CVECTOR	VR_HLT_EXP	= {48,16,8};
//static CVECTOR	VR_HLT_TAB	= {0,80,96};
static CVECTOR	VR_HLT_TAB	= {80,10,10};
//コア
static CVECTOR	VR_CORE_DEF	= {10,96,64};
static CVECTOR	VR_CORE_EXP	= {90,32,64};
//static CVECTOR	VR_CORE_TAB	= {10,160,192};
//static CVECTOR	VR_CORE_TAB	= {127,20,20};
static CVECTOR	VR_CORE_TAB	= {180,40,40};


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_IRREACTION|DG_FLAG_ONEPIECE)
//#define	HLT_OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
#define	HLT_OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_NOFOG)
#define APEAR_TIME_A (30)
#define APEAR_TIME_B (30)
#define FADE_TIME (APEAR_TIME_A+APEAR_TIME_B)

#define	HOMING_SIGHT_WIDE	(250.0f)

#define		BODY_SIZE_X		(500.0f)
#define		BODY_SIZE_Y		(500.0f)
#define		BODY_SIZE_Z		(500.0f)
#define		BODY_SIZE		(500.0f)


#define LV1_NUM (1)
#define LV2_NUM (2)
#define LV3_NUM (10)

#define OCT_LV1_NUM	(2)
#define OCT_LV2_NUM (4)
#define OCT_LV3_NUM (4)

#define STR_LV1_NUM (1)
#define STR_LV2_NUM (2)
#define STR_LV3_NUM (2)

#define		CUBE_03_LV1_NUM	(2)
#define		CUBE_03_LV2_NUM	(8)
#define		CUBE_03_LV3_NUM	(1)


//#define	SPEED01	(30.0f)
//#define	SPEED01	(1000.0f)
//#define	SPEED01	(500.0f)
#define	SPEED01	(250.0f)

#define APPEAR_SEC		(0.20f)

extern int BP_FRAMES_PER_SEC();
#define APPEAR_SCALE_SPEED	(1.0f/(BP_FRAMES_PER_SEC()*APPEAR_SEC))

#define TRG_VIEW
#define P_TRG_VIEW

void VR_AllTrgSkip(VR_TRG_Work *work){
	int i;
//	GM_SetTargetSize( &work->b_trg, &DG_ZeroVector ) ;
	work->b_trg.class |= TARGET_SKIP ;
	if(work->mine_trg != NULL){
		work->mine_trg->class |= TARGET_SKIP ;
	}
	for(i=0;i<work->trg01_num;i++){
//		GM_SetTargetSize( &work->core_trg01[i], &DG_ZeroVector ) ;
		work->core_trg01[i].class |= TARGET_SKIP ;
	}
	for(i=0;i<work->trg02_num;i++){
//		GM_SetTargetSize( &work->core_trg02[i], &DG_ZeroVector ) ;
		work->core_trg02[i].class |= TARGET_SKIP ;
	}
	for(i=0;i<work->trg03_num;i++){
//		GM_SetTargetSize( &work->core_trg03[i], &DG_ZeroVector ) ;
		work->core_trg03[i].class |= TARGET_SKIP ;
	}
}
static void VR_FlrCheck(VR_TRG_Work *work){
	if(!( work->type & VR_TARGET_FL_MARK)){
		return ;
	}
	if(HZX_LevelHazardCheck( work->control.hzx_id,(FVECTOR *) &(work->efe_world.m[3]),
		( HZX_CHK_F_FLOOR ),(HZX_FLOOR_NO_PLAYER) )){ 
		float fl_level ;
		fl_level = HZX_GetFloorLevel() ;
		if(fl_level > -20000.0f){
			work->fl_pos.vx = work->efe_world.m[3][0] ;
			work->fl_pos.vy = HZX_GetFloorLevel() ;
			work->fl_pos.vz = work->efe_world.m[3][2] ;
			return ;
		}
	}
	work->fl_pos.vx = -1000000.0f ;
	work->fl_pos.vy = -1000000.0f ;
	work->fl_pos.vz = -1000000.0f ;
}
static void VR_UnSetAllTrgSkip(VR_TRG_Work *work){
	int i;
#if 0
	if(work->type & VR_TARGET_TYPE_PUNCH){
		work->b_trg.class &= ~TARGET_SKIP ;
		return ;
	}
#else
	work->b_trg.class &= ~TARGET_SKIP ;
#endif
	for(i=0;i<work->trg01_num;i++){
		work->core_trg01[i].class &= ~TARGET_SKIP ;
	}
	for(i=0;i<work->trg02_num;i++){
		work->core_trg02[i].class &= ~TARGET_SKIP ;
	}
	for(i=0;i<work->trg03_num;i++){
		work->core_trg03[i].class &= ~TARGET_SKIP ;
	}
}

void	VR_CheckMessage(VR_TRG_Work	*work )
{
    GV_MSG	*msg ;
	int n_msg, code ;
    n_msg = work->control.n_msg ;
	msg = work->control.msg ;

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case VR_TRG_MSG_WAKE :
				if(work->type & VR_TARGET_TYPE_WAIT){
					work->type &= ~VR_TARGET_TYPE_WAIT ;
					VR_UnSetAllTrgSkip(work) ;
					work->b_trg.class |= TARGET_LOCKON ;
					if(work->hlt_objs != NULL ){
						DG_VisibleObjs(work->hlt_objs) ;
					}
					UNSET_FLAG( work->hom_trg.status, HOMING_SKIP ) ;
					work->visi_cnt = DIRECT_TICK(FADE_TIME) ;

printf("VR_TRG_MSG_WAKE RECI!!!!!!!!!!!!!!!\n");
				}
				break;
			case VR_TRG_MSG_DISAPPEAR :
				work->status |=TRG_ST_DISAPPEAR  ;
				break;
		}
		msg++ ;
	}
}


extern int BP_AdjustTick(int);
#define OL_SPEED (BP_AdjustTick(7))

static void VR_TRG_OL_COL(VR_TRG_Work *work,GV_ACT_EX *actor){

	if(work->ol_col.col[0].cd <= 0 ) {
		return ;
	}
	if(work->ol_col.col[0].cd >OL_SPEED) {
		work->ol_col.col[0].cd -= OL_SPEED ;
		GV_CallChildSignalFunc( actor,OL_SIG_SetColor, (int) &work->ol_col ) ;
	}else if(work->ol_col.col[0].cd >0) {
		work->ol_col.col[0].cd = 0 ;
		GV_CallChildSignalFunc( actor,OL_SIG_SetColor, (int) &work->ol_col ) ;
	}
}
static void TRG_Clash_SE_KATANA(FVECTOR *pos ){
	//破壊音 通常とＮＧの自動呼び分け
	int secode = SD_A_V_CLASHS ; /*ノーマルＳＥ*/
	{
		if(VR_COMBO_CNT >= 0) {
			if((VR_COMBO_CHAIN - 1) >= VR_COMBO_LIST_MAX){
				/*ＭＡＸコンボ*/
				secode = SD_A_V_CLASHU ;
			}else {
				/*コンボ*/
				secode = SD_A_V_CLASHT ;
			}
		}else {
			/*ノーマルＳＥ*/
		}
	}

	GM_SeSetMode(secode,pos,GM_SEMODE_BOMB) ;

}

void VR_trglifecheck(VR_TRG_Work *work,GV_ACT_EX *actor){
	int i;
   FVECTOR	pos = { 0 };
	char str2[16] ;

	VR_TRG_OL_COL(work,actor) ;
	if(work->blast_cnt>0){
		work->blast_cnt-- ;
		return ;
	}
	if((work->vital >= VR_TARGET01_LIFE_MAX)||(work->status &TRG_ST_DISAPPEAR)){
		if(!(work->status &TRG_ST_DESTROY)){
			if(work->vital >= VR_TARGET01_LIFE_MAX){
/*破壊時処理*/
			GV_MatToVec( &work->body.objs->world, &pos ) ;
			/*機雷エフェクト消し*/
				if(work->mine_act != NULL){
					GV_DestroyOtherActor(work->mine_act) ;
				}
				//アウトライン表示
				work->ol_col.col[0].cd = 127 ;
				if(work->type & VR_TARGET_TYPE_KATANA){
					if(work->type & VR_TARGET_TYPE_NG){
						if(work->status & TRG_ST_FAINTBLADE ){
						}else {
							work->finish = VR_TrgHitFunc(work->str_work,
								&work->b_trg.center,&work->b_trg.center,&work->b_trg.power->force,
									VR_TRG_NG,VR_TypeFlagToNum(work->type)) ;
						}
					}else {
						work->finish = VR_TrgHitFunc(work->str_work,
							&work->b_trg.center,&work->b_trg.center,&work->b_trg.power->force,
							VR_TRG_KATANA,VR_TypeFlagToNum(work->type)) ;
					}
				}
				if(work->type & VR_TARGET_TYPE_BLAST){
//					work->blast_cnt = BLAST_DELAY ;
//誘爆
printf("VR_BLAST_RANGE[%d]\n",VR_BLAST_RANGE);
					NewBlast3( &pos, BOTH_SIDE, VR_BLAST_RANGE, VR_BLAST_RANGE, 20, FNT_BLAST, WP_Grenade, 
//						BLAST_TYPE_NO_NOISE|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER) ;
						BLAST_TYPE_NO_NOISE|BLAST_TYPE_WALLTHROUGH) ;
				}else {
//白分解
					if(work->ef_mdl_code != 0){
						CallBreakObj(work->ef_mdl_code,&work->body.objs->world) ;
					}
				}
//コンボチェック
				if(work->type & (VR_TARGET_TYPE_MINE)){
				}else {
/*破壊ＳＥは必ずコンボ更新の前に行う*/
					if(work->type & (VR_TARGET_TYPE_KATANA)){
						if(work->last_weapon_type & (WP_BLADESTAB|WP_BLADE)){
							/*斬り*/
							TRG_Clash_SE_KATANA(&pos) ;
						}else if(work->status & TRG_ST_FAINTBLADE ){
							/*峰討ち*/
							if(work->type & VR_TARGET_TYPE_NG){
								GM_SeSetMode(SD_A_V_HITOK1,&pos,GM_SEMODE_BOMB) ;
								GM_SeSetMode(SD_A_V_CLASHA,&pos,GM_SEMODE_BOMB) ;
							}else {
								VR_TRG_Clash_SE(&pos, work->type) ;
							}
						}else {
							VR_TRG_Clash_SE(&pos, work->type) ;
						}
					}else {
						VR_TRG_Clash_SE(&pos, work->type) ;
					}
					if(work->type & (VR_TARGET_TYPE_DARK_OCTA)){
						/*NOP*/
					}else if(work->type & VR_TARGET_TYPE_NG){
						if(work->status & TRG_ST_FAINTBLADE ){
						}else {
							VR_ResetCombo(&work->b_trg.center) ;
							VR_AddScore(VR_TRG_SCORE_NG) ;
							sprintf(str2,"NG!!%d\n",VR_TRG_SCORE_NG) ;
							CALL_SIG_3DPrintf(work->str_work,&work->b_trg.center ,str2,1,DIRECT_TICK(60)) ;
						}
					}else {
						VR_DestoryComboCheck(work->finish,&work->b_trg.center) ;
						VR_AddScore(work->finish) ;
						sprintf(str2,"%d\n",work->finish) ;
						CALL_SIG_3DPrintf(work->str_work,&work->b_trg.center ,str2,0,VR_COMBO_LIMIT) ;
					}
				}
			}else if(!(work->type&VR_TARGET_TYPE_WAIT)){

//消滅時の処理 破壊ではない NG的回避用
				extern void *NewSigBreakObj2(int mdlcode,FMATRIX *world,int mode ,
					float max_dis,int time_a,int time_b ,int *sw) ;
//飛び散りではなく崩れるように
				NewSigBreakObj2(work->mdl_code,&work->efe_world,(0x02)|(0x04)|(0x08)|(0x10)|(0x0100)|(0x0200),
//					2000.0f,10,30,&work->build_sw) ;
					500.0f,30,15,&work->build_sw) ;
				GM_SeSetMode(SD_A_V_SIMATO,&pos,GM_SEMODE_BOMB) ;
			}
//アタリ消滅
			VR_AllTrgSkip(work) ;
			work->b_trg.class &= ~TARGET_LOCKON ;
			GM_SetTargetSize( &work->b_trg, &DG_ZeroVector ) ;
			for(i=0;i<work->trg01_num;i++){
				GM_SetTargetSize( &work->core_trg01[i], &DG_ZeroVector ) ;
				work->core_trg01[i].class |= TARGET_SKIP ;
			}
			for(i=0;i<work->trg02_num;i++){
				GM_SetTargetSize( &work->core_trg02[i], &DG_ZeroVector ) ;
				work->core_trg02[i].class |= TARGET_SKIP ;
			}
			for(i=0;i<work->trg03_num;i++){
				GM_SetTargetSize( &work->core_trg03[i], &DG_ZeroVector ) ;
				work->core_trg03[i].class |= TARGET_SKIP ;
			}
			work->b_trg.class |= TARGET_SKIP ;

			if(!(work->type & VR_TARGET_TYPE_NG)){
				VR_ClearTarget() ;
			}
			DG_InvisibleObjs(work->body.objs) ;
			if(work->hlt_objs != NULL ){
				DG_InvisibleObjs(work->hlt_objs) ;
			}
			SET_FLAG( work->hom_trg.status, HOMING_SKIP ) ;
			if(work->proc_id != 0){
				VR_ExecProcName(work->proc_id,work->name) ;
				work->proc_id= 0;
			}
			work->sight = 0 ;
			GM_RadarResetFlag( &work->rctrl, RADAR_VISIBLE ) ;
			work->build_sw = -1 ;
			if(work->fl_pos_act != NULL){
				GV_DestroyOtherActor( work->fl_pos_act ) ;
				work->fl_pos_act = NULL ;
				work->type &= ~VR_TARGET_FL_MARK ;
			}
			work->status |= TRG_ST_DESTROY ;
		}
	} 
}
#if 0
void VR_checkhoming(VR_TRG_Work *work){
	if(work->hom_trg.status & HOMING_YOU){
		/*維持*/
		work->sight = 1 ;
	}else if((!(work->hom_trg.status & HOMING_YOU))) {
		/*終了*/
		work->sight = 0 ;
	}
	work->hom_trg.status &= ~HOMING_YOU ;
}
#endif
void VR_CheckWakeEffect(VR_TRG_Work *work){
	FVECTOR	pos ;


	if(work->visi_cnt == DIRECT_TICK(FADE_TIME)){
extern void *NewSigBreakObj2(int mdlcode,FMATRIX *world,int mode ,float max_dis,int time_a,int time_b ,int *sw) ;

		if(work->type & VR_TARGET_NO_APPEAR_EF){
			work->scale = 0.0f;
		}else {
			if(!(work->type&VR_TARGET_STEALTH)){
				NewSigBreakObj2(work->mdl_code,&work->efe_world,(0x1200),
				2000.0f,APEAR_TIME_A ,APEAR_TIME_B ,&work->build_sw) ;
			}
		}
		GV_MatToVec( &work->body.objs->world, &pos ) ;
		GM_SeSetMode(SD_A_V_MATO_G,&pos,GM_SEMODE_BOMB) ;
	}
}

/*ライフ減らしたあとにやること*/
static void SetDarkTargetSize(VR_TRG_Work *work){
	FVECTOR trg_scale ;

	if(work->type & (VR_TARGET_TYPE_DARK_OCTA)){
		work->dmg_scale = (1.0f - TRG_DARK_SCALE_MIN) * (VR_TARGET01_LIFE_MAX - work->vital) 
				/ VR_TARGET01_LIFE_MAX  + TRG_DARK_SCALE_MIN ;
		if(work->dmg_scale < TRG_DARK_SCALE_MIN){
			work->dmg_scale = TRG_DARK_SCALE_MIN ;
		}
printf("DMG TARGET SCALE [%f]\n",work->dmg_scale);
		trg_scale.vx = trg_scale.vy = trg_scale.vz = TRG_DARK_SIZE * work->dmg_scale / 2.0f;
		GM_SetTargetSize( &work->b_trg, &trg_scale ) ;
		GM_SetTargetSize( &work->core_trg01[0], &trg_scale ) ;
	}
}
static void VR_SeekTrgCall(VR_TRG_Work *work,TARGET *def){
//	char str[16];
	work->seek_num = def->name ;
//printf("seek_num [%d]\n",work->seek_num);
//	sprintf(str,"SEEK[%d]!!",work->seek_num) ;
}
static	void	ChildTargCallBack_Parent( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	VR_TRG_Work	*work ;
//	char str[24] ;
	work = ( VR_TRG_Work * )ptr ;
	work->last_weapon_type = def->weapon_type ;
//	str = "hit" ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
#if 0
		 	if( def->weapon_type & (WP_BULLET|WP_M92|WP_BLADALL)){
#else
		 	if( 1){
#endif
//				sprintf(str,"hitpos %d",(int)def->hit.vy) ;
//				sprintf(str,"PARENT") ;
			}
		}

		if(def->weapon_type & (WP_BLAST|WP_NIKITA)){
			VR_AllTrgSkip(work) ;
//			printf("WP_BLAST HIT\n") ;
			work->finish = VR_TrgHitFunc(work->str_work,&work->b_trg.center,&work->b_trg.center,
				&work->b_trg.power->force,
				VR_TRG_BLAST,VR_TypeFlagToNum(work->type)) ;
			work->vital= 255 ;
			work->blast_cnt = BLAST_DELAY ;
		}
		/*ライフ減らしたあとにやること*/
		SetDarkTargetSize(work) ;
	}
	def->weapon_type = 0 ;
}
static	void	ChildTargCallBack_NG( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	VR_TRG_Work	*work ;
//	char str[24] ;
	work = ( VR_TRG_Work * )ptr ;
//	str = "hit" ;
	work->last_weapon_type = def->weapon_type ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			if(def->weapon_type & (WP_BULLET|WP_M92|WP_BLAST|WP_BLADESTAB|WP_BLADE)){
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&work->b_trg.center,
					&off->power->force,VR_TRG_NG,VR_TypeFlagToNum(work->type)) ;
				work->vital= 255 ;
			}
		}
	}
	if(def->weapon_type & WP_LASERSIGHT) {
		VR_SeekTrgCall(work,def) ;
	}

	if(def->weapon_type & (WP_BLAST|WP_NIKITA)){
		work->blast_cnt = BLAST_DELAY ;
	}
#if 0
//カタナNGには専用コールバック用意したので非殺傷武器は無視
//殺さず武器なら殺さずフラグ立ててから破壊
	if( def->weapon_type & (WP_PUNCHR|WP_PUNCHL|WP_KICK|WP_KICK1)){
		GM_SeSetMode(SD_A_V_MTDMGP,&work->control.mov,GM_SEMODE_BOMB) ;
	}
	if( def->weapon_type & (WP_PUNCHR|WP_PUNCHL|WP_BLADEFAINT|WP_KICK|WP_KICK1)){
		GM_SeSetMode(SD_A_V_MTDMGP,&work->control.mov,GM_SEMODE_BOMB) ;
		work->status |= TRG_ST_FAINTBLADE ;
	}
#endif

	if(def->weapon_type & (WP_BLAST|WP_NIKITA)){
		work->blast_cnt = BLAST_DELAY ;
	}

	def->weapon_type = 0 ;
}

static void FlyingTarget(VR_TRG_Work *work ,TARGET *off,TARGET *def){
//蹴られて飛ぶ
	GM_SeSetMode(SD_A_V_MTDMGK,&work->control.mov,GM_SEMODE_BOMB) ;
	work->control.skip_flag = 
		(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_ONLINE_CHECK|CTRL_SKIP_TRAP); 

	if(work->type & VR_TARGET_FLY_RAISE){
		work->control.mov.vy += 1.0f ;
	}
	work->control.hzx_check_type = ~(HZX_SEG_NO_MISSILE) ;
	if(work->mode !=TRG_BOUND_MODE){
		work->mode_buf = work->mode ;
	}
	work->mode = TRG_BOUND_MODE ;
	work->b_trg.class |= TARGET_SKIP ;
//	work->cnt = 0 ;
	{
		FVECTOR	vec ;
		if(def->weapon_type & (WP_NOPLAYER)){
			GV_SubVec3F( &def->center, &off->center, &vec ) ;
		}else {
			if ( GM_PlayerStatus & PLAYER_ROLLING){
				SVECTOR rot ;
				//ローリング中は進行方向へ飛ばす
//				DG_COPY_VEC( &vec, &(GM_PlayerControl->step) ) ;
				rot = DG_ZeroSVector ;
				rot.vy = GM_PlayerControl->rot.vy ;
				vec = DG_ZeroVector ;
				vec.vz = 1000.0f ;
				DG_SetPos2(&DG_ZeroVector,&rot);
				DG_RotVector(&vec,&vec,1) ;
			}else {
				GV_SubVec3F( &def->center, &GM_PlayerPosition, &vec ) ;
			}
		}
		vec.vy = 0.0f ;
	    _sceVu0Normalize( &vec, &vec ) ;
	    _sceVu0ScaleVector( &vec, &vec, work->punch_force ) ;
		work->control.step.vx = vec.vx ;
		work->control.step.vy = 0.0f ;
		work->control.step.vz = vec.vz ;
	}

	if(work->type & VR_TARGET_FLY_PROC_CALL){
		if(work->proc_id != 0){
			VR_ExecProcName(work->proc_id,work->name) ;
			work->proc_id= 0;
		}
	}
}
static	void	ChildTargCallBack_Katana( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	VR_TRG_Work	*work ;
//	char str[24] ;
	work = ( VR_TRG_Work * )ptr ;
	work->last_weapon_type = def->weapon_type ;
	if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			if( def->weapon_type & (WP_BLADEFAINT)){
	//パンチ扱い
				work->amb_cnt = VR_TRG_FLS_CNT_KATANA ;
//	パンチ2発分
				work->vital += 2*(VR_GetPartsDmg(VR_TRG_TYPE_KATANA,1)); 
				if(work->vital >= VR_TARGET01_LIFE_MAX){
//カタナ的は斬りと殴りで破壊音が違うのでここで呼び分け
					GM_SeSetMode(SD_A_V_HITOK1,&work->control.mov,GM_SEMODE_BOMB) ;
				}else {
//殴られ音	破壊時にはダメージ音はいらんらしい
					GM_SeSetMode(SD_A_V_MTDMGP,&work->control.mov,GM_SEMODE_BOMB) ;
				}
			}else if( def->weapon_type & (WP_PUNCHR|WP_PUNCHL)){
//パンチ
				work->amb_cnt = VR_TRG_FLS_CNT_KATANA ;
				work->vital += VR_GetPartsDmg(VR_TRG_TYPE_KATANA,1) ; 
				if(work->vital >= VR_TARGET01_LIFE_MAX){
//カタナ的は斬りと殴りで破壊音が違うのでここで呼び分け
					GM_SeSetMode(SD_A_V_HITOK1,&work->control.mov,GM_SEMODE_BOMB) ;
				}else {
//殴られ音	破壊時にはダメージ音はいらんらしい
					GM_SeSetMode(SD_A_V_MTDMGP,&work->control.mov,GM_SEMODE_BOMB) ;
				}
			}else if(def->weapon_type & (WP_KICK|WP_KICK1)){
//蹴られて飛ぶ
				FlyingTarget( work ,off, def) ;
			}else if(def->weapon_type & (WP_BLADESTAB|WP_BLADE)){
//斬られ破壊音
				work->vital = VR_TRG_LIFE_MAX; 
//				TRG_Clash_SE_KATANA(&work->control.mov) ;
				GM_SeSetMode(SD_A_V_HITOK1,&work->control.mov,GM_SEMODE_BOMB) ;
				work->vital = VR_TRG_LIFE_MAX ; 
			}
		}
	}
	def->weapon_type = 0 ;
}
static	void	ChildTargCallBack_Katana_NG( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	VR_TRG_Work	*work ;
//	char str[24] ;
	work = ( VR_TRG_Work * )ptr ;
	work->last_weapon_type = def->weapon_type ;

	if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			if( def->weapon_type & (WP_BLADEFAINT)){
//峰撃ち 必ず破壊
				work->vital = VR_TRG_LIFE_MAX; 
				GM_SeSetMode(SD_A_V_CLASHA,&work->control.mov,GM_SEMODE_BOMB) ;
				work->status |= TRG_ST_FAINTBLADE ;
			}else if( def->weapon_type & (WP_PUNCHR|WP_PUNCHL)){
//パンチ
				work->amb_cnt = VR_TRG_FLS_CNT_KATANA ;
				work->vital += VR_GetPartsDmg(VR_TRG_TYPE_KATANA,1) ; 
				if(work->vital >= VR_TARGET01_LIFE_MAX){
//カタナ的は斬りと殴りで破壊音が違うのでここで呼び分け
					GM_SeSetMode(SD_A_V_CLASHA,&work->control.mov,GM_SEMODE_BOMB) ;
					GM_SeSetMode(SD_A_V_HITNG1,&work->control.mov,GM_SEMODE_BOMB) ;
				}else {
//殴られ音	破壊時にはダメージ音はいらんらしい
					GM_SeSetMode(SD_A_V_MTDMGP,&work->control.mov,GM_SEMODE_BOMB) ;
				}
			}else if(def->weapon_type & (WP_KICK|WP_KICK1)){
//蹴られて飛ぶ
				FlyingTarget( work ,off,def) ;
			}else if(def->weapon_type & (WP_BLADESTAB|WP_BLADE)){
//斬られ破壊音
				work->vital = VR_TRG_LIFE_MAX; 
				GM_SeSetMode(SD_A_V_CLASHS,&work->control.mov,GM_SEMODE_BOMB) ;
				GM_SeSetMode(SD_A_V_HITNG1,&work->control.mov,GM_SEMODE_BOMB) ;
printf("NG!!!\n");
			}
		}
	}
	def->weapon_type = 0 ;
}

static	void	ChildTargCallBack_01( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	VR_TRG_Work	*work ;
//	char str[24] ;
	work = ( VR_TRG_Work * )ptr ;
//	str = "hit" ;
	work->last_weapon_type = def->weapon_type ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
//			def->power->force = off->power->force ;
			if(def->weapon_type & (WP_BULLET|WP_M92|WP_BLADALL)){
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&work->b_trg.center,
				&off->power->force,VR_TRG_LEVEL1,VR_TypeFlagToNum(work->type)) ;
				work->vital+= VR_GetPartsDmg(VR_TypeFlagToNum(work->type),1) ; 
				work->amb_cnt = VR_TRG_FLS_CNT ;

			}
			if(work->vital < VR_TARGET01_LIFE_MAX){
				GM_SeSetMode(SD_A_V_MTHIBI,&def->hit,GM_SEMODE_BOMB) ;
			}
			/*ライフ減らしたあとにやること*/
			SetDarkTargetSize(work) ;
		}
	}
	if(def->weapon_type & WP_LASERSIGHT) {
		VR_SeekTrgCall(work,def) ;
	}
	if(def->weapon_type & (WP_BLAST|WP_NIKITA)){
		work->blast_cnt = BLAST_DELAY ;
	}
	def->weapon_type = 0 ;
}

static	void	ChildTargCallBack_02( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	VR_TRG_Work	*work ;
//	char str[24] ;
	work = ( VR_TRG_Work * )ptr ;
//	str = "hit" ;
	work->last_weapon_type = def->weapon_type ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
//			def->power->force = off->power->force ;
			if(def->weapon_type & (WP_BULLET|WP_M92|WP_BLADALL)){
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&work->b_trg.center,
				&off->power->force,VR_TRG_LEVEL2,VR_TypeFlagToNum(work->type)) ;
				work->vital+= VR_GetPartsDmg(VR_TypeFlagToNum(work->type),2) ; 
				work->amb_cnt = VR_TRG_FLS_CNT ;
				if(work->vital < VR_TARGET01_LIFE_MAX){
					GM_SeSetMode(SD_A_V_MTHIBI,&def->hit,GM_SEMODE_BOMB) ;
				}
			}
		}
	}
	if(def->weapon_type & WP_LASERSIGHT) {
		VR_SeekTrgCall(work,def) ;
	}
	if(def->weapon_type & (WP_BLAST|WP_NIKITA)){
		work->blast_cnt = BLAST_DELAY ;
	}
	def->weapon_type = 0 ;
}
static void vr_setflagall_objs(DG_OBJS *objs,int flag){
	int i ;
	for(i= 0 ;i<objs->n_models;i++){
		objs->objs[i].flag |= flag ;
	}
}
static void vr_unsetflagall_objs(DG_OBJS *objs,int flag){
	int i ;
	for(i= 0 ;i<objs->n_models;i++){
		objs->objs[i].flag &= ~flag ;
	}
}

static	void	ChildTargCallBack_03( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	VR_TRG_Work	*work ;
//	char str[24] ;
	work = ( VR_TRG_Work * )ptr ;
//	str = "hit" ;
	work->last_weapon_type = def->weapon_type ;
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
//			def->power->force = off->power->force ;
			if(def->weapon_type & (WP_BULLET|WP_M92|WP_BLADALL)){
				work->finish = VR_TrgHitFunc(work->str_work,&def->hit,&work->b_trg.center,
				&off->power->force,VR_TRG_LEVEL3,VR_TypeFlagToNum(work->type)) ;
				work->vital+= VR_GetPartsDmg(VR_TypeFlagToNum(work->type),3) ; 
				work->amb_cnt = VR_TRG_FLS_CNT ;
				if(work->vital < VR_TARGET01_LIFE_MAX){
					GM_SeSetMode(SD_A_V_MTHIBI,&def->hit,GM_SEMODE_BOMB) ;
				}
			}
		}
	}
	if(def->weapon_type & WP_LASERSIGHT) {
		VR_SeekTrgCall(work,def) ;
	}
	if(def->weapon_type & (WP_BLAST|WP_NIKITA)){
		work->blast_cnt = BLAST_DELAY ;
	}
	def->weapon_type = 0 ;
}

static void SwitchHideCheck(VR_TRG_Work *work,int hide_cycle ,int appear_cycle ){

	if(work->status & TRG_ST_HIDE){
		/*隠れ中*/
		work->scale_cnt++ ;
		if(work->scale_cnt>=hide_cycle){
			work->scale_cnt = 0 ;
			work->status &= ~TRG_ST_HIDE ;
		}
	}else {
		/*表示中*/
		if((work->amb_cnt<=0)&&(work->mode != TRG_BOUND_MODE)){
			/*ダメージ直後と吹っ飛び中は消えない*/
//&&(work->trg_w.type & VR_TARGET_TYPE_KATANA)
			work->scale_cnt++ ;
			if(work->scale_cnt>=appear_cycle){
				work->scale_cnt = 0 ;
				work->status |= TRG_ST_HIDE ;
			}
		}
	}
}
static void HideScaleCheck(VR_TRG_Work *work){

	if((work->status & TRG_ST_HIDE)&&(work->mode != TRG_BOUND_MODE)){
		if(work->scale == 1.0f) {
			GM_SeSetMode(SD_A_V_MATO_O,&work->control.mov,GM_SEMODE_NORMAL) ;
		}
		if(work->scale>0.0f) work->scale -= APPEAR_SCALE_SPEED ;
		if(work->scale<0.0f) work->scale = 0.0f;
		if(work->type & VR_TARGET_TYPE_NG){
			VR_AllTrgSkip(work) ;
		}else {
			if(work->scale == 0.0f){
				VR_AllTrgSkip(work) ;
				SET_FLAG( work->hom_trg.status, HOMING_SKIP ) ;
			}
		}
	}else {
		if(work->mode == TRG_BOUND_MODE){
			VR_AllTrgSkip(work) ;
			work->scale = 1.0f ;
		}
		if(work->scale == 0.0f) {
			GM_SeSetMode(SD_A_V_MATO_I,&work->control.mov,GM_SEMODE_NORMAL) ;
		}
		if(work->scale<1.0f) work->scale += APPEAR_SCALE_SPEED;
		if(work->scale>1.0f) work->scale = 1.0f;
		if(work->mode != TRG_BOUND_MODE ){
			if(work->type & VR_TARGET_TYPE_NG){
				if(work->scale == 1.0f){
					VR_UnSetAllTrgSkip(work) ;
					UNSET_FLAG( work->hom_trg.status, HOMING_SKIP ) ;
				}
			}else {
				VR_UnSetAllTrgSkip(work) ;
				UNSET_FLAG( work->hom_trg.status, HOMING_SKIP ) ;
			}
		}
	}
}
void VR_movetrg(VR_TRG_Work *work , FVECTOR *t_pos){
	FVECTOR scale ;
	int i;
	float fade ;
	int tmp ;

//	SVECTOR rot ;

/*
	待機中 死亡 出現エフェクト中は非表示
	ただし出現エフェクト無しフラグの場合は代わりにスケールで登場するので
	表示有効
*/
//	printf("visi_cnt [%d]\n",work->visi_cnt );
	
	if(
	((work->visi_cnt == 0 )
	||((work->visi_cnt > 0 )&&(work->type & VR_TARGET_NO_APPEAR_EF)))
	&&(!(work->status & TRG_ST_DESTROY ))
	&&(!(work->type & VR_TARGET_TYPE_WAIT))
	&&(!(GM_VRStatus & GM_VR_IDLE))
	){
//	printf("DG_VisibleObjs\n");
		DG_VisibleObjs(work->body.objs) ;
		if(work->hlt_objs != NULL ){
			DG_VisibleObjs(work->hlt_objs) ;
		}
		if(work->scale_cycle != 0){
			SwitchHideCheck(work,work->scale_cycle,work->appear_cycle) ;
		}else if(work->type & (VR_TARGET_HIDESTART|VR_TARGET_VISIBLESTART)){

			if((work->type & VR_TARGET_HIDESTART )&&(work->type & VR_TARGET_VISIBLESTART)){
				printf("VR_TARGET_HIDESTART & VR_TARGET_VISIBLESTART BOTH SET ERROR!!!!\n");
				//	ASSERT(0) ;
			}
			if(VR_TARGET_HIDE_CYCLE != 0){
				SwitchHideCheck(work,VR_TARGET_HIDE_CYCLE,VR_TARGET_APPEAR_CYCLE) ;
			}
		}
		HideScaleCheck(work) ;
		GM_RadarSetFlag( &work->rctrl, RADAR_VISIBLE ) ;
		if(( work->type & VR_TARGET_FL_MARK)&&(!(GM_VRStatus & GM_VR_IDLE))){
			VR_FlrCheck(work) ;
			if(work->fl_pos_act == NULL) {
				extern void *NewVRFloorMarker2(FVECTOR *mov,float scale);
				GV_SetActorChild((GV_ACT_EX *) work->p_act,
				work->fl_pos_act = (void *)NewVRFloorMarker2(&work->fl_pos,1.0f) );
			}
		}
	}else {
		DG_InvisibleObjs(work->body.objs) ;
		if(work->hlt_objs != NULL ){
			DG_InvisibleObjs(work->hlt_objs) ;
		}
		GM_RadarResetFlag( &work->rctrl, RADAR_VISIBLE ) ;
	}

#if 0
	if(work->status & TRG_ST_HIDE){
		if(work->scale>0.0f) work->scale -= APPEAR_SCALE_SPEED ;
		if(work->scale<0.0f) work->scale = 0.0f;
	}else {
		if(work->scale<1.0f) work->scale += APPEAR_SCALE_SPEED;
		if(work->scale>1.0f) work->scale = 1.0f;
	}
#endif
//	work->scale = 1.0f;
	
	scale.vx = scale.vy = scale.vz = work->scale ;
	scale.vw = 1.0f ;

	{
		FMATRIX	mat;
		SVECTOR	rot ;
		FVECTOR	vec;
		rot.vx = 0 ;

		if(work->type & VR_TARGET_TYPE_KATANA){
			if(work->mode == TRG_BOUND_MODE ){
				SVECTOR rot ;

            if ( BP_IsPAL()==TRUE )
   				work->bound_cnt += 360;
            else
				   work->bound_cnt += 300;

            if(!(work->status & TRG_ST_DESTROY)){
					if(work->bound_cnt > 4095){
						GM_SeSetMode(SD_A_V_ROLLIN,t_pos,GM_SEMODE_BOMB) ;
					}
				}
				work->bound_cnt &= 4095;
				rot.vx = 1024 ; 
				rot.vy = work->bound_cnt ; 
				rot.vz = 0 ; 
				DG_SetPos2(t_pos,&rot) ;

			}else {
				rot.vx = 0 ; 
				if(work->type &VR_TARGET_X_ROT){
					rot.vx = 3072 ; 
				}else {
					rot.vx = 0 ; 
				}
//				rot.vz = 0 ; 
				rot.vz = ((int)((float)work->cnt*work->jiten)+work->dir_off)&4095 ;
//				rot.vy = ((int)((float)work->cnt*work->jiten)+work->dir_off)&4095 ;

				if(work->katana_dir <0 ){
					rot.vy = work->control.rot.vy ;
				}else {
					rot.vy = work->katana_dir ;
				}

				DG_SetPos2(t_pos,&rot) ;
			}
		}else {

			if(work->type & VR_TARGET_TYPE_DIR){
				rot.vy = work->control.rot.vy ;
			}else if(work->type & VR_TARGET_TYPE_P_DIR){
				rot.vy = work->dir_off ;
			}else {	
				rot.vy = ((int)((float)work->cnt*work->jiten)+work->dir_off)&4095 ;
			}
			rot.vz = 0 ;
			DG_SetPos2(t_pos,&rot);
		}


		DG_GetPos(&mat) ;
		if(!(work->status & TRG_ST_DESTROY)){
			GM_MoveTarget2( &work->b_trg, &mat ) ;
			for(i=0;i<work->trg01_num;i++){
				GM_MoveTarget2( &work->core_trg01[i], &mat ) ;
			}
			for(i=0;i<work->trg02_num;i++){
				GM_MoveTarget2( &work->core_trg02[i], &mat ) ;
			}
			for(i=0;i<work->trg03_num;i++){
				GM_MoveTarget2( &work->core_trg03[i], &mat ) ;
			}
		}
		DG_GetPos(&work->efe_world) ;
		DG_ScalePos(&scale) ;

		if(work->type & (VR_TARGET_TYPE_DARK_OCTA)){
			if(work->dmg_scale != 1.0f) {
				FVECTOR dmg_scale ;
				dmg_scale.vx = dmg_scale.vy = dmg_scale.vz = work->dmg_scale ;
				dmg_scale.vw = 1.0f ;
				DG_ScalePos(&dmg_scale) ;
			}
		}

		DG_PutObjs( work->body.objs );
		if(work->hlt_objs != NULL ){
			DG_PutObjs( work->hlt_objs );
		}
		DG_PutVector(&work->hom_shift,&vec,1) ;
		DG_SetPos2(&vec,&DG_ZeroSVector) ;
		DG_GetPos(&work->hom_mat) ;
	}
#if 0
	if(!(work->status & TRG_ST_DESTROY )){
		if( work->type & VR_TARGET_FL_MARK){
			VR_FlrCheck(work) ;
		}
		if(( work->type & VR_TARGET_FL_MARK)&&(work->fl_pos_act == NULL)) {
			extern void *NewVRFloorMarker2(FVECTOR *mov,float scale);
			GV_SetActorChild((GV_ACT_EX *) work->p_act,
			work->fl_pos_act = (void *)NewVRFloorMarker2(&work->fl_pos,1.0f) );
		}
	}
#endif

/*シーク中のオブジェを表示*/
	if(work->hlt_objs != NULL ){
		vr_setflagall_objs(work->hlt_objs,DG_FLAG_INVISIBLE) ;
		if(!(work->status & TRG_ST_DESTROY)){
			if(work->visi_cnt > 0 ){
				vr_unsetflagall_objs(work->hlt_objs,DG_FLAG_INVISIBLE) ;
				if(work->visi_cnt >DIRECT_TICK(15)) {
					tmp = 63.0f ;
				}else {
					tmp = 63.0f *(float)work->visi_cnt/16.0f ;
				}
				fade = (float) tmp / 127.0f ;
				work->hlt_lights[1].m[ 3 ][ 0 ] = (float) work->core_rgb.r *fade;
				work->hlt_lights[1].m[ 3 ][ 1 ] = (float) work->core_rgb.g *fade;
				work->hlt_lights[1].m[ 3 ][ 2 ] = (float) work->core_rgb.b *fade;
			}else if(work->seek_num != -1){
				if(work->seek_num >= work->hlt_objs->n_models){
					printf("seek_num [%d] n_models[%d]\n",work->seek_num, work->hlt_objs->n_models) ;
				}
				ASSERT(work->seek_num < work->hlt_objs->n_models) ;
				work->hlt_objs->objs[work->seek_num].flag &= ~DG_FLAG_INVISIBLE ;

				work->hlt_lights[1].m[ 3 ][ 0 ] = (float) work->hlt_rgb.r ;
				work->hlt_lights[1].m[ 3 ][ 1 ] = (float) work->hlt_rgb.g ;
				work->hlt_lights[1].m[ 3 ][ 2 ] = (float) work->hlt_rgb.b ;
			}else {
				for(i=0;i<work->hlt_objs->n_models;i++){
					if(work->core_mdl & (1<<i)){
						work->hlt_objs->objs[i].flag &= ~DG_FLAG_INVISIBLE ;
					}
				}
				if(work->type & VR_TARGET_TYPE_NG){

               if ( BP_IsPAL()==TRUE )
   					tmp = (GM_StagePlayTime*6/5) & 31 ;
               else
					   tmp = (GM_StagePlayTime) & 31 ;

               if(tmp> 15 ) {
						tmp = 31 - tmp ;
					}
					fade = (float) tmp / 31.0f ;
				}else {
               if ( BP_IsPAL()==TRUE )
   					tmp = (GM_StagePlayTime*6/5) & 127 ;
               else
					   tmp = (GM_StagePlayTime) & 127 ;

               if(tmp> 63 ) {
						tmp = 127 - tmp ;
					}
					fade = (float) tmp / 63.0f ;
				}
				work->hlt_lights[1].m[ 3 ][ 0 ] = (float) work->core_rgb.r *fade;
				work->hlt_lights[1].m[ 3 ][ 1 ] = (float) work->core_rgb.g *fade;
				work->hlt_lights[1].m[ 3 ][ 2 ] = (float) work->core_rgb.b *fade;
			}
			work->seek_num = -1 ;
		}
	}
	if(work->mine_trg != NULL){
		GM_MoveTargetMap( work->mine_trg, t_pos ,work->control.map) ;
		GM_PutTarget( work->mine_trg );
//		NewTargetView2( work->mine_trg, 255, 0, 0 ) ;
	}


#if 0
/* debug バウンド表示 */
	NewBoundingBoxView_1( work->body.objs->world, 
							&work->body.objs->bound_min,
							&work->body.objs->bound_max, 0x8000ffff );
#endif

}


static void MulFvec(FVECTOR* fv, float scale){
	fv->vx*=scale ;
	fv->vy*=scale ;
	fv->vz*=scale ;
}

static inline void setfvec(FVECTOR *fv,float x,float y,float z){
	fv->vx = x ;
	fv->vy = y ;
	fv->vz = z ;
}
void VR_SetTargetOne(VR_TRG_Work *work)
{
	int flag,map;
	FVECTOR	core_size01[1] ;
	FVECTOR	off_set01[1] ;

	FVECTOR	b_size = { 500.0f, 500.0f, 500.0f ,1.0f} ;
	FVECTOR	nkt_b_size = { 440.0f, 440.0f, 440.0f ,1.0f} ;

	work->trg01_num = 1 ;
	work->trg02_num = 0 ;
	work->trg03_num = 0 ;

	work->core_trg01 = GV_Malloc( sizeof(TARGET)*work->trg01_num ) ;
	work->core_trg02 = NULL ;
	work->core_trg03 = NULL ;

	work->core_mdl = (1<<0) ;


/*LV1*/
	setfvec(&off_set01[0],0.0f,0.0f,0.0f);
	setfvec(&core_size01[0],1000.0f,1000.0f,1000.0f);

	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);
	GM_CurrentMap =	map = work->control.map;
	MulFvec(&core_size01[0], VR_TARGET01_SCALE) ;

//	GM_SetTarget(&work->b_trg,(flag|TARGET_CHILD_ALWAYS),map,ENEMY_SIDE,&b_size,&DG_ZeroVector);
	GM_SetTarget(&work->b_trg,(flag|TARGET_CHILD_ALWAYS),map,ENEMY_SIDE,&nkt_b_size,&DG_ZeroVector);

	GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
	GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );
	MulFvec(&core_size01[0], 0.5f*VR_TARGET01_SCALE) ;
	MulFvec(&off_set01[0], VR_TARGET01_SCALE) ;
	GM_SetTarget( &work->core_trg01[0], flag,map, ENEMY_SIDE, &core_size01[0], &off_set01[0] ) ;
	GM_SetTargetParts( &work->b_trg, &work->core_trg01[0],1, 0 ) ;
	if(work->type & VR_TARGET_TYPE_NG){
		GM_SetTargetCallBack( &work->core_trg01[0],(TARGET_CALLBACK) ChildTargCallBack_NG, work ) ;
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
	}else {
		GM_SetTargetCallBack(&work->core_trg01[0], (TARGET_CALLBACK)ChildTargCallBack_01, work ) ;
		GM_SetTargetCallBack( &work->b_trg,(TARGET_CALLBACK) ChildTargCallBack_Parent, work ) ;
	}
	GM_SetTargetWeaponType(&work->core_trg01[0],0);
	GM_SetPowerTarget(&work->core_trg01[0],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		NewTargetView( &work->core_trg01[0], 255, 0, 0 ) ;
		NewTargetView( &work->b_trg, 0, 255, 0 ) ;
	}
#endif
}
#define NOV_TRI_NUM	(3)
void VR_SetTargetOneTri(VR_TRG_Work *work)
{
	int flag,map;
	int i;
	FVECTOR	b_size = { 500.0f, 500.0f, 60.0f,1.0f} ;
	FVECTOR	core_size01[NOV_TRI_NUM] ;
	FVECTOR	off_set01[NOV_TRI_NUM] ;
	short	seek_num01[NOV_TRI_NUM] ;

	work->trg01_num = NOV_TRI_NUM ;
	work->trg02_num = 0 ;
	work->trg03_num = 0 ;

	work->core_trg01 = GV_Malloc( sizeof(TARGET)*work->trg01_num ) ;
	work->core_trg02 = NULL ;
	work->core_trg03 = NULL ;

	work->core_mdl = (1<<0) ;


/*LV1*/

	setfvec(&off_set01[0],	0.0f,205.0f,0.0f);
	setfvec(&core_size01[0],980.0f,290.0f,120.0f) ;
	seek_num01[0] = -1;

	setfvec(&off_set01[1],	0.0f,-85.0f,0.0f);
	setfvec(&core_size01[1],780.0f,290.0f,120.0f) ;
	seek_num01[1] = -1;

	setfvec(&off_set01[2],	0.0f,-375.0f,0.0f);
	setfvec(&core_size01[2],420.0f,290.0f,120.0f) ;
	seek_num01[2] = -1;


	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);
	GM_CurrentMap =	map = work->control.map;
//	MulFvec(&core_size01[0], VR_TARGET01_SCALE) ;
	MulFvec(&b_size, VR_TARGET01_SCALE) ;
	GM_SetTarget(&work->b_trg,(flag|TARGET_CHILD_ALWAYS),map,ENEMY_SIDE,&b_size,&DG_ZeroVector);
	GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
	GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );

	for(i=0;i<work->trg01_num;i++){
		MulFvec(&core_size01[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set01[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg01[i], flag,map, ENEMY_SIDE, &core_size01[i], &off_set01[i] ) ;
	}
	GM_SetTargetParts( &work->b_trg, &work->core_trg01[0],work->trg01_num, 0 ) ;
	if(work->type & VR_TARGET_TYPE_NG){
		for(i=0;i<work->trg01_num;i++){
			GM_SetTargetCallBack( &work->core_trg01[i], (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
		}
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
	}else {
		for(i=0;i<work->trg01_num;i++){
			GM_SetTargetCallBack(&work->core_trg01[i], (TARGET_CALLBACK)ChildTargCallBack_01, work ) ;
		}
		GM_SetTargetCallBack( &work->b_trg,(TARGET_CALLBACK) ChildTargCallBack_Parent, work ) ;
	}
	for(i=0;i<work->trg01_num;i++){
		GM_SetTargetWeaponType(&work->core_trg01[i],0);
		GM_SetPowerTarget(&work->core_trg01[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
	}
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		for(i=0;i<work->trg01_num;i++){
			NewTargetView( &work->core_trg01[i], 255, 0, 0 ) ;
		}
		NewTargetView( &work->b_trg, 255, 127, 0 ) ;
	}
#endif
}
void VR_SetTargetHex(VR_TRG_Work *work)
{
	int flag,map;
	FVECTOR	core_size01[1] ;
	FVECTOR	off_set01[1] ;

	work->trg01_num = 1 ;
	work->trg02_num = 0 ;
	work->trg03_num = 0 ;

	work->core_trg01 = GV_Malloc( sizeof(TARGET)*work->trg01_num ) ;
	work->core_trg02 = NULL ;
	work->core_trg03 = NULL ;

	work->core_mdl = (1<<0) ;


/*LV1*/
	setfvec(&off_set01[0],0.0f,0.0f,0.0f);
	setfvec(&core_size01[0],2000.0f,1100.0f,2000.0f);

	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);
	GM_CurrentMap =	map = work->control.map;
	MulFvec(&core_size01[0], VR_TARGET01_SCALE) ;
	GM_SetTarget(&work->b_trg,(flag|TARGET_CHILD_ALWAYS|TARGET_LOCKON),map,ENEMY_SIDE,&core_size01[0],&DG_ZeroVector);
	GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
	GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );
	MulFvec(&core_size01[0], 0.5f*VR_TARGET01_SCALE) ;
	MulFvec(&off_set01[0], VR_TARGET01_SCALE) ;
	GM_SetTarget( &work->core_trg01[0], flag,map, ENEMY_SIDE, &core_size01[0], &off_set01[0] ) ;
	GM_SetTargetParts( &work->b_trg, &work->core_trg01[0],1, 0 ) ;
	if(work->type & VR_TARGET_TYPE_NG){
		GM_SetTargetCallBack( &work->core_trg01[0], (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
	}else {
		GM_SetTargetCallBack(&work->core_trg01[0],(TARGET_CALLBACK) ChildTargCallBack_01, work ) ;
		GM_SetTargetCallBack( &work->b_trg,(TARGET_CALLBACK) ChildTargCallBack_Parent, work ) ;
	}
	GM_SetTargetWeaponType(&work->b_trg,0);
	GM_SetPowerTarget(&work->core_trg01[0],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		NewTargetView( &work->core_trg01[0], 255, 0, 0 ) ;
	}
#endif
}

void VR_SetTargetKatana(VR_TRG_Work *work)
{
	int flag,map;
	int i;
	FVECTOR	b_size = { 130.0f, 1000.0f, 110.0f ,1.0f} ;
	FVECTOR	core_size01[STR_LV1_NUM] ;
	FVECTOR	core_size02[STR_LV2_NUM] ;
	FVECTOR	core_size03[STR_LV3_NUM] ;
	FVECTOR	off_set01[STR_LV1_NUM] ;
	FVECTOR	off_set02[STR_LV2_NUM] ;
	FVECTOR	off_set03[STR_LV3_NUM] ;

	work->trg01_num = STR_LV1_NUM ;
	work->trg02_num = STR_LV2_NUM ;
	work->trg03_num = STR_LV3_NUM ;

	work->core_trg01 = GV_Malloc( sizeof(TARGET)*work->trg01_num ) ;
	work->core_trg02 = GV_Malloc( sizeof(TARGET)*work->trg02_num ) ;
	work->core_trg03 = GV_Malloc( sizeof(TARGET)*work->trg03_num ) ;

	work->core_mdl = (1<<0) ;

/*LV3*/
	setfvec(&off_set03[0],0.0f,670.0f,20.0f);
	setfvec(&core_size03[0],260.0f,600.0f,220.0f);

	setfvec(&off_set03[1],0.0f,-670.0f,20.0f);
	setfvec(&core_size03[1],260.0f,600.0f,220.0f);


/*LV2*/
	setfvec(&off_set02[0],0.0f,235,20.0f);
	setfvec(&core_size02[0],260.0f,270.0f,220.0f);

	setfvec(&off_set02[1],0.0f,-235,20.0f);
	setfvec(&core_size02[1],260.0f,270.0f,220.0f);

/*LV1*/
	setfvec(&off_set01[0],0.0f,0.0f,20.0f);
	setfvec(&core_size01[0],260.0f,200.0f,220.0f);


	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON|TARGET_ROTATE|TARGET_POWER);
	GM_CurrentMap =	map = work->control.map;
	MulFvec(&b_size, VR_TARGET01_SCALE) ;
	GM_SetTarget( &work->b_trg, (flag|TARGET_CHILD_ALWAYS),map, ENEMY_SIDE, &b_size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
    GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );

	for(i=0;i<work->trg01_num;i++){
		MulFvec(&core_size01[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set01[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg01[i], flag,map, ENEMY_SIDE, &core_size01[i], &off_set01[i] ) ;
	}
	for(i=0;i<work->trg02_num;i++){
		MulFvec(&core_size02[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set02[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg02[i], flag,map, ENEMY_SIDE, &core_size02[i], &off_set02[i] ) ;
	}
	for(i=0;i<work->trg03_num;i++){
		MulFvec(&core_size03[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set03[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg03[i], flag,map, ENEMY_SIDE, &core_size03[i], &off_set03[i] ) ;
	}

	if(work->type & VR_TARGET_TYPE_NG){
		GM_SetTargetCallBack( &work->b_trg,(TARGET_CALLBACK) ChildTargCallBack_Katana_NG, work ) ;
		for(i=0;i<work->trg01_num;i++){
			GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_Katana_NG, work ) ;
		}
		for(i=0;i<work->trg02_num;i++){
			GM_SetTargetCallBack( &work->core_trg02[i], (TARGET_CALLBACK)ChildTargCallBack_Katana_NG, work ) ;
		}
		for(i=0;i<work->trg03_num;i++){
			GM_SetTargetCallBack( &work->core_trg03[i], (TARGET_CALLBACK)ChildTargCallBack_Katana_NG, work ) ;
		}
	}else {
//		GM_SetTargetCallBack( &work->b_trg,(TARGET_CALLBACK) ChildTargCallBack_Parent, work ) ;
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_Katana, work ) ;
		for(i=0;i<work->trg01_num;i++){
//			GM_SetTargetCallBack( &work->core_trg01[i], (TARGET_CALLBACK)ChildTargCallBack_01, work ) ;
			GM_SetTargetCallBack( &work->core_trg01[i], (TARGET_CALLBACK)ChildTargCallBack_Katana, work ) ;
		}
		for(i=0;i<work->trg02_num;i++){
//			GM_SetTargetCallBack( &work->core_trg02[i], (TARGET_CALLBACK)ChildTargCallBack_02, work ) ;
			GM_SetTargetCallBack( &work->core_trg02[i],(TARGET_CALLBACK) ChildTargCallBack_Katana, work ) ;
		}
		for(i=0;i<work->trg03_num;i++){
//			GM_SetTargetCallBack( &work->core_trg03[i],(TARGET_CALLBACK) ChildTargCallBack_03, work ) ;
			GM_SetTargetCallBack( &work->core_trg03[i], (TARGET_CALLBACK)ChildTargCallBack_Katana, work ) ;
		}
	}
	for(i=0;i<work->trg01_num;i++){
		GM_SetTargetWeaponType(&work->core_trg01[i],0);
		GM_SetPowerTarget(&work->core_trg01[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg01[i],1, 0 ) ;
	}
	for(i=0;i<work->trg02_num;i++){
		GM_SetTargetWeaponType(&work->core_trg02[i],0);
		GM_SetPowerTarget(&work->core_trg02[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg02[i],1, 1 ) ;
	}
	for(i=0;i<work->trg03_num;i++){
		GM_SetTargetWeaponType(&work->core_trg03[i],0);
		GM_SetPowerTarget(&work->core_trg03[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg03[i],1, 2 ) ;
	}
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		NewTargetView( &work->b_trg, 255, 0, 0 ) ;
		for(i=0;i<work->trg01_num;i++){
			NewTargetView( &work->core_trg01[i], 255, 0, 0 ) ;
		}
		for(i=0;i<work->trg02_num;i++){
			NewTargetView( &work->core_trg02[i], 255, 255, 0 ) ;
		}
		for(i=0;i<work->trg03_num;i++){
			NewTargetView( &work->core_trg03[i], 255, 255, 255 ) ;
		}
	}
#endif
}
void VR_SetTargetTri02(VR_TRG_Work *work)
{
	int flag,map;
	int i;
	FVECTOR	b_size = { BODY_SIZE, BODY_SIZE_Y, 25.0f ,1.0f} ;
	FVECTOR	core_size01[LV1_NUM] ;
	FVECTOR	core_size02[LV2_NUM] ;
	FVECTOR	core_size03[LV3_NUM] ;
	FVECTOR	off_set01[LV1_NUM] ;
	FVECTOR	off_set02[LV2_NUM] ;
	FVECTOR	off_set03[LV3_NUM] ;

	short	seek_num01[LV1_NUM] ;
	short	seek_num02[LV2_NUM] ;
	short	seek_num03[LV3_NUM] ;

	work->core_mdl = (1<<5) ;

	work->trg01_num = LV1_NUM ;
	work->trg02_num = LV2_NUM ;
	work->trg03_num = LV3_NUM ;

	work->core_trg01 = GV_Malloc( sizeof(TARGET)*work->trg01_num ) ;
	work->core_trg02 = GV_Malloc( sizeof(TARGET)*work->trg02_num ) ;
	work->core_trg03 = GV_Malloc( sizeof(TARGET)*work->trg03_num ) ;

	ASSERT(work->core_trg01 != NULL) ;
	ASSERT(work->core_trg02 != NULL) ;
	ASSERT(work->core_trg03 != NULL) ;

	//core
	setfvec(&off_set01[0], 		0.0f,	0.0f,	0.0f) ;
//	setfvec(&core_size01[0],	240.0f,220.0f,100.0f) ;
	setfvec(&core_size01[0],	240.0f,220.0f,5.0f) ;
	seek_num01[0] = 5;


	setfvec(&off_set02[0],	0.0f,0.0f,0.0f);
//	setfvec(&core_size02[0],340.0f,520.0f,60.0f) ;
	setfvec(&core_size02[0],340.0f,520.0f,10.0f) ;
	seek_num02[0] = 4;

	setfvec(&off_set02[1],	0.0f,25.0f,0.0f);
//	setfvec(&core_size02[1],500.0f,230.0f,60.0f) ;
	setfvec(&core_size02[1],500.0f,230.0f,10.0f) ;
	seek_num02[1] = 4;



	setfvec(&off_set03[0],	320.0f,180.0f,0.0f) ;
	setfvec(&core_size03[0],340.0f,300.0f,80.0f) ;
	seek_num03[0] = 1;

	setfvec(&off_set03[1],	-320.0f,180.0f,0.0f) ;
	setfvec(&core_size03[1],340.0f,300.0f,80.0f) ;
	seek_num03[1] = 3;

	setfvec(&off_set03[2],	0.0f,-365.0f,0.0f) ;
	setfvec(&core_size03[2],340.0f,290.0f,80.0f) ;
	seek_num03[2] = 2;

	setfvec(&off_set03[3],	0.0f,310.0f,0.0f) ;
	setfvec(&core_size03[3],460.0f,100.0f,60.0f) ;
	seek_num03[3] = 0;

	setfvec(&off_set03[4],	335.0f,-30.0f,0.0f) ;
	setfvec(&core_size03[4],170.0f,120.0f,60.0f) ;
	seek_num03[4] = 0;
	
	setfvec(&off_set03[5],	-335.0f,-30.0f,0.0f) ;
	setfvec(&core_size03[5],170.0f,120.0f,60.0f) ;
	seek_num03[5] = 0;
	
	setfvec(&off_set03[6],	270.0f,-155.0f,0.0f) ;
	setfvec(&core_size03[6],200.0f,130.0f,60.0f) ;
	seek_num03[6] = 0;
	
	setfvec(&off_set03[7],	-270.0f,-155.0f,0.0f) ;
	setfvec(&core_size03[7],200.0f,130.0f,60.0f) ;
	seek_num03[7] = 0;

	setfvec(&off_set03[8],	230.0f,-290.0f,0.0f) ;
	setfvec(&core_size03[8],120.0f,140.0f,60.0f) ;
	seek_num03[8] = 0;
	
	setfvec(&off_set03[9],	-230.0f,-290.0f,0.0f) ;
	setfvec(&core_size03[9],120.0f,140.0f,60.0f) ;
	seek_num03[9] = 0;


	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON|TARGET_ROTATE|TARGET_POWER);
	GM_CurrentMap =	map = work->control.map;
	MulFvec(&b_size, VR_TARGET01_SCALE) ;
//	GM_SetTarget( &work->b_trg, (flag|TARGET_CHILD_ALWAYS),map, ENEMY_SIDE, &b_size, &DG_ZeroVector ) ;
	GM_SetTarget( &work->b_trg, (flag|TARGET_CHILD_ALWAYS),map, ENEMY_SIDE, &b_size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
    GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );

	for(i=0;i<LV1_NUM;i++){
		MulFvec(&core_size01[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set01[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg01[i], flag,map, ENEMY_SIDE, &core_size01[i], &off_set01[i] ) ;
		work->core_trg01[i].name = seek_num01[i] ;
	}
	for(i=0;i<LV2_NUM;i++){
		MulFvec(&core_size02[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set02[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg02[i], flag,map, ENEMY_SIDE, &core_size02[i], &off_set02[i] ) ;
		work->core_trg02[i].name = seek_num02[i] ;
	}
	for(i=0;i<LV3_NUM;i++){
		MulFvec(&core_size03[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set03[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg03[i], flag,map, ENEMY_SIDE, &core_size03[i], &off_set03[i] ) ;
		work->core_trg03[i].name = seek_num03[i] ;
	}

	if(work->type & VR_TARGET_TYPE_NG){
		GM_SetTargetCallBack( &work->b_trg,(TARGET_CALLBACK) ChildTargCallBack_NG, work ) ;
		for(i=0;i<LV1_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_NG, work ) ;
		}
		for(i=0;i<LV2_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg02[i],(TARGET_CALLBACK) ChildTargCallBack_NG, work ) ;
		}
		for(i=0;i<LV3_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg03[i], (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
		}
	}else {
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_Parent, work ) ;
		for(i=0;i<LV1_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_01, work ) ;
		}
		for(i=0;i<LV2_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg02[i], (TARGET_CALLBACK)ChildTargCallBack_02, work ) ;
		}
		for(i=0;i<LV3_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg03[i],(TARGET_CALLBACK) ChildTargCallBack_03, work ) ;
		}
	}
	for(i=0;i<LV1_NUM;i++){
		GM_SetTargetWeaponType(&work->core_trg01[i],0);
		GM_SetPowerTarget(&work->core_trg01[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg01[i],1, 0 ) ;
	}
	for(i=0;i<LV2_NUM;i++){
		GM_SetTargetWeaponType(&work->core_trg02[i],0);
		GM_SetPowerTarget(&work->core_trg02[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg02[i],1, 1 ) ;
	}
	for(i=0;i<LV3_NUM;i++){
		GM_SetTargetWeaponType(&work->core_trg03[i],0);
		GM_SetPowerTarget(&work->core_trg03[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg03[i],1, 2 ) ;
	}
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		NewTargetView( &work->b_trg, 255, 0, 0 ) ;

		for(i=0;i<LV1_NUM;i++){
			NewTargetView( &work->core_trg01[i], 255, 0, 0 ) ;
		}
		for(i=0;i<LV2_NUM;i++){
			NewTargetView( &work->core_trg02[i], 255, 255, 0 ) ;
		}
		for(i=0;i<LV3_NUM;i++){
			NewTargetView( &work->core_trg03[i], 255, 255, 255 ) ;
		}
	}
#endif
}

#if 0
void VR_SetTargetPunch(VR_TRG_Work *work)
{
	int flag,map;
	FVECTOR	b_size = { 250.0f, 250.0f, 250.0f ,1.0f} ;
	MulFvec(&b_size, VR_TARGET01_SCALE) ;

	/*子ターゲット無し*/
	work->trg01_num = 0 ;
	work->trg02_num = 0 ;
	work->trg03_num = 0 ;

	work->core_mdl = (1<<3)|(1<<4) ;

	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON);
	GM_CurrentMap =	map = work->control.map;
	GM_SetTarget( &work->b_trg, (flag),map, ENEMY_SIDE, &b_size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
	GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );
	if(work->type & VR_TARGET_TYPE_NG){
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
	}else {
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_Katana, work ) ;
	}
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		if(work->type & VR_TARGET_TYPE_NG){
			NewTargetView( &work->b_trg, 0, 0, 255 ) ;
		}else {
			NewTargetView( &work->b_trg, 255, 0, 0 ) ;
		}
	}
#endif
}
#endif



void VR_SetTargetOct(VR_TRG_Work *work)
{
	int flag,map;
	int i;
	FVECTOR	b_size = { BODY_SIZE, BODY_SIZE_Y, BODY_SIZE_Z ,1.0f} ;
	FVECTOR	core_size01[OCT_LV1_NUM+OCT_LV2_NUM+OCT_LV3_NUM] ;
	FVECTOR	off_set01[OCT_LV1_NUM+OCT_LV2_NUM+OCT_LV3_NUM] ;

	short	seek_num01[OCT_LV1_NUM+OCT_LV2_NUM+OCT_LV3_NUM] ;

	work->trg01_num = OCT_LV1_NUM+OCT_LV2_NUM+OCT_LV3_NUM ;
	work->trg02_num = 0 ;
	work->trg03_num = 0 ;

	work->core_trg01 = GV_Malloc( sizeof(TARGET)*work->trg01_num ) ;

	ASSERT(work->core_trg01 != NULL) ;

	work->core_mdl = (1<<3)|(1<<4) ;


	setfvec(&off_set01[0],0.0f,0.0f,0.0f);
	setfvec(&core_size01[0],520.0f,880.0f,240.0f);
	seek_num01[0] = 0 ;

	setfvec(&off_set01[1],0.0f,0.0f,0.0f);
	setfvec(&core_size01[1],920.0f,520.0f,240.0f);
	seek_num01[1] = 0 ;

	setfvec(&off_set01[2],0.0f,0.0f,0.0f);
	setfvec(&core_size01[2],420.0f,680.0f,380.0f);
	seek_num01[2] = 0 ;

	setfvec(&off_set01[3],0.0f,0.0f,0.0f);
	setfvec(&core_size01[3],720.0f,420.0f,380.0f);
	seek_num01[3] = 0 ;

	setfvec(&off_set01[4],0.0f,0.0f,265.0f);
	setfvec(&core_size01[4],320.0f,580.0f,150.0f);
	seek_num01[4] = 1 ;

	setfvec(&off_set01[5],0.0f,0.0f,265.0f);
	setfvec(&core_size01[5],620.0f,320.0f,150.0f);
	seek_num01[5] = 1 ;

	setfvec(&off_set01[6],0.0f,0.0f,-265.0f);
	setfvec(&core_size01[6],320.0f,580.0f,150.0f);
	seek_num01[6] = 2 ;

	setfvec(&off_set01[7],0.0f,0.0f,-265.0f);
	setfvec(&core_size01[7],620.0f,320.0f,150.0f);
	seek_num01[7] = 2 ;

	setfvec(&off_set01[8],0.0f,0.0f,410.0f);
	setfvec(&core_size01[8],300.0f,300.0f,140.0f);
	seek_num01[8] = 3 ;

	setfvec(&off_set01[9],0.0f,0.0f,-410.0f);
	setfvec(&core_size01[9],300.0f,300.0f,140.0f);
	seek_num01[9] = 4 ;

	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON|TARGET_ROTATE|TARGET_POWER);
	GM_CurrentMap =	map = work->control.map;
	MulFvec(&b_size, VR_TARGET01_SCALE) ;
	GM_SetTarget( &work->b_trg, (flag|TARGET_CHILD_ALWAYS|TARGET_LOCKON),map,
//	GM_SetTarget( &work->b_trg, (flag|TARGET_CHILD_ALWAYS|TARGET_LOCKON),map,
		ENEMY_SIDE, &b_size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
	GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		NewTargetView( &work->b_trg , 255, 0, 0 ) ;
	}
#endif
	for(i=0;i<work->trg01_num;i++){
		MulFvec(&core_size01[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set01[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg01[i], flag,map, ENEMY_SIDE, &core_size01[i], &off_set01[i] ) ;
		work->core_trg01[i].name = seek_num01[i] ;
	}

	if(work->type & VR_TARGET_TYPE_NG){

		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;

		for(i=0;i<(OCT_LV1_NUM+OCT_LV2_NUM+OCT_LV3_NUM);i++){
			if(i<OCT_LV3_NUM) {

				GM_SetTargetCallBack( &work->core_trg01[i], (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
#ifdef DEBUG_MODE
				if(work->type & VR_TARGET_DEBUG){
					NewTargetView( &work->core_trg01[i], 0, 0, 255 ) ;
				}
#endif
			}else if(i<(OCT_LV3_NUM+OCT_LV2_NUM)){
				GM_SetTargetCallBack( &work->core_trg01[i], (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
#ifdef DEBUG_MODE
				if(work->type & VR_TARGET_DEBUG){
					NewTargetView( &work->core_trg01[i], 0, 255, 0 ) ;
				}
#endif
			}else {
				GM_SetTargetCallBack( &work->core_trg01[i], (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
#ifdef DEBUG_MODE

				if(work->type & VR_TARGET_DEBUG){
					NewTargetView( &work->core_trg01[i], 255, 0, 0 ) ;
				}
#endif
			}
		}


	}else {
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_Parent, work ) ;
#if 1
		for(i=0;i<(OCT_LV1_NUM+OCT_LV2_NUM+OCT_LV3_NUM);i++){
			if(i<OCT_LV3_NUM) {

				GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_03, work ) ;
#ifdef DEBUG_MODE
				if(work->type & VR_TARGET_DEBUG){
					NewTargetView( &work->core_trg01[i], 0, 0, 255 ) ;
				}
#endif

			}else if(i<(OCT_LV3_NUM+OCT_LV2_NUM)){
				GM_SetTargetCallBack( &work->core_trg01[i], (TARGET_CALLBACK)ChildTargCallBack_02, work ) ;
#ifdef DEBUG_MODE
				if(work->type & VR_TARGET_DEBUG){
					NewTargetView( &work->core_trg01[i], 0, 255, 0 ) ;
				}
#endif
			}else {
				GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_01, work ) ;
#ifdef DEBUG_MODE
				if(work->type & VR_TARGET_DEBUG){
					NewTargetView( &work->core_trg01[i], 255, 0, 0 ) ;
				}
#endif
			}
		}
#else
		for(i=0;i<OCT_LV1_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_01, work ) ;
		}
		for(i=OCT_LV1_NUM;i<OCT_LV1_NUM+OCT_LV2_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_02, work ) ;
		}
		for(i=OCT_LV1_NUM+OCT_LV2_NUM;i<OCT_LV1_NUM+OCT_LV2_NUM+OCT_LV3_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_03, work ) ;
		}
#endif
	}
	for(i=0;i<work->trg01_num;i++){
		GM_SetTargetWeaponType(&work->core_trg01[i],0);
		GM_SetPowerTarget(&work->core_trg01[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
	}
	GM_SetTargetParts( &work->b_trg, &work->core_trg01[0],work->trg01_num, 0 ) ;
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		NewTargetView( &work->b_trg, 255, 255, 255 ) ;
	}
#endif
}



void VR_SetTargetCross(VR_TRG_Work *work){

	int flag,map;
	int i;
	FVECTOR	b_size = { BODY_SIZE, BODY_SIZE_Y, BODY_SIZE_Z ,1.0f} ;
	FVECTOR	core_size01[CUBE_03_LV1_NUM] ;
	FVECTOR	core_size02[CUBE_03_LV2_NUM] ;
	FVECTOR	core_size03[CUBE_03_LV3_NUM] ;
	FVECTOR	off_set01[CUBE_03_LV1_NUM] ;
	FVECTOR	off_set02[CUBE_03_LV2_NUM] ;
	FVECTOR	off_set03[CUBE_03_LV3_NUM] ;

	short	seek_num01[CUBE_03_LV1_NUM] ;
	short	seek_num02[CUBE_03_LV2_NUM] ;
	short	seek_num03[CUBE_03_LV3_NUM] ;


	work->core_mdl = (1<<5) ;

	work->trg01_num = CUBE_03_LV1_NUM;
	work->trg02_num = CUBE_03_LV2_NUM;
	work->trg03_num = CUBE_03_LV3_NUM;


	work->core_trg01 = GV_Malloc( sizeof(TARGET)*CUBE_03_LV1_NUM ) ;
	work->core_trg02 = GV_Malloc( sizeof(TARGET)*CUBE_03_LV2_NUM ) ;
	work->core_trg03 = GV_Malloc( sizeof(TARGET)*CUBE_03_LV3_NUM ) ;

	ASSERT(work->core_trg01 != NULL) ;
	ASSERT(work->core_trg02 != NULL) ;
	ASSERT(work->core_trg03 != NULL) ;


/*LV2*/
	setfvec(&off_set03[0],0.0f,0.0f,0.0f);
	setfvec(&core_size03[0],1000.0f,1000.0f,150.0f);
	seek_num03[0] = 4 ;

/*LV3*/
	setfvec(&off_set02[0],310.0f,385,0.0f);
	setfvec(&core_size02[0],380.0f,230.0f,150.0f);
	seek_num02[0] = 0 ;

	setfvec(&off_set02[1],385.0f,195.0f,0.0f);
	setfvec(&core_size02[1],230.0f,150.0f,150.0f);
	seek_num02[1] = 0 ;

	setfvec(&off_set02[2],310.0f,-385,0.0f);
	setfvec(&core_size02[2],380.0f,230.0f,150.0f);
	seek_num02[2] = 1 ;

	setfvec(&off_set02[3],385,-195,0.0f);
	setfvec(&core_size02[3],230.0f,150.0f,150.0f);
	seek_num02[3] = 1 ;

	setfvec(&off_set02[4],-310.0f,-385,0.0f);
	setfvec(&core_size02[4],380.0f,230.0f,150.0f);
	seek_num02[4] = 2 ;

	setfvec(&off_set02[5],-385,-195,0.0f);
	setfvec(&core_size02[5],230.0f,150.0f,150.0f);
	seek_num02[5] = 2 ;

	setfvec(&off_set02[6],-310.0f,385,0.0f);
	setfvec(&core_size02[6],380.0f,230.0f,150.0f);
	seek_num02[6] = 3 ;

	setfvec(&off_set02[7],-385,195,0.0f);
	setfvec(&core_size02[7],230.0f,150.0f,150.0f);
	seek_num02[7] = 3 ;

/*LV1*/
	setfvec(&off_set01[0],0.0f,0.0f,0.0f);
//	setfvec(&core_size01[0],1000.0f,120.0f,150.0f);
	setfvec(&core_size01[0],1000.0f,120.0f,5.0f);
	seek_num01[0] = 5 ;

	setfvec(&off_set01[1],0.0f,0.0f,0.0f);
//	setfvec(&core_size01[1],120.0f,1000.0f,150.0f);
	setfvec(&core_size01[1],120.0f,1000.0f,5.0f);
	seek_num01[1] = 5 ;


	/*ターゲット設定*/

	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);

	GM_CurrentMap =	map = work->control.map;

	MulFvec(&b_size, VR_TARGET01_SCALE) ;
	GM_SetTarget( &work->b_trg, (flag|TARGET_CHILD_ALWAYS|TARGET_LOCKON),map, ENEMY_SIDE, &b_size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
    GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );

	for(i=0;i<CUBE_03_LV1_NUM;i++){
		MulFvec(&core_size01[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set01[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg01[i], flag,map, ENEMY_SIDE, &core_size01[i], &off_set01[i] ) ;
		work->core_trg01[i].name = seek_num01[i] ;
	}
	for(i=0;i<CUBE_03_LV2_NUM;i++){
		MulFvec(&core_size02[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set02[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg02[i], flag,map, ENEMY_SIDE, &core_size02[i], &off_set02[i] ) ;
		work->core_trg02[i].name = seek_num02[i] ;
	}
	for(i=0;i<CUBE_03_LV3_NUM;i++){
		MulFvec(&core_size03[i], 0.5f*VR_TARGET01_SCALE) ;
		MulFvec(&off_set03[i], VR_TARGET01_SCALE) ;
		GM_SetTarget( &work->core_trg03[i], flag,map, ENEMY_SIDE, &core_size03[i], &off_set03[i] ) ;
		work->core_trg03[i].name = seek_num03[i] ;
	}

	if(work->type & VR_TARGET_TYPE_NG){
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
		for(i=0;i<CUBE_03_LV1_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_NG, work ) ;
		}
		for(i=0;i<CUBE_03_LV2_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg02[i],(TARGET_CALLBACK) ChildTargCallBack_NG, work ) ;
		}
		for(i=0;i<CUBE_03_LV3_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg03[i], (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
		}
	}else {
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_Parent, work ) ;
		for(i=0;i<CUBE_03_LV1_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg01[i],(TARGET_CALLBACK) ChildTargCallBack_01, work ) ;
		}
		for(i=0;i<CUBE_03_LV2_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg02[i],(TARGET_CALLBACK) ChildTargCallBack_03, work ) ;
		}
		for(i=0;i<CUBE_03_LV3_NUM;i++){
			GM_SetTargetCallBack( &work->core_trg03[i], (TARGET_CALLBACK)ChildTargCallBack_02, work ) ;
		}
	}
	for(i=0;i<CUBE_03_LV1_NUM;i++){
		GM_SetTargetWeaponType(&work->core_trg01[i],0);
		GM_SetPowerTarget(&work->core_trg01[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg01[i],1, 0 ) ;
	}
	for(i=0;i<CUBE_03_LV2_NUM;i++){
		GM_SetTargetWeaponType(&work->core_trg02[i],0);
		GM_SetPowerTarget(&work->core_trg02[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg02[i],1, 1 ) ;
	}
	for(i=0;i<CUBE_03_LV3_NUM;i++){
		GM_SetTargetWeaponType(&work->core_trg03[i],0);
		GM_SetPowerTarget(&work->core_trg03[i],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
		GM_SetTargetParts( &work->b_trg, &work->core_trg03[i],1, 2 ) ;
	}
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		if(work->type & VR_TARGET_TYPE_NG){
			NewTargetView( &work->core_trg03[0], 0, 0, 255 ) ;
			NewTargetView( &work->core_trg03[1], 0, 0, 255 ) ;
		}else {
			for(i=0;i<CUBE_03_LV1_NUM;i++){
				NewTargetView( &work->core_trg01[i], 255, 0, 0 ) ;
			}
			for(i=0;i<CUBE_03_LV2_NUM;i++){
				NewTargetView( &work->core_trg02[i], 255, 255, 0 ) ;
			}
			for(i=0;i<CUBE_03_LV3_NUM;i++){
				NewTargetView( &work->core_trg03[i], 255, 255, 255 ) ;
			}
		}
	}
#endif
}

void VR_SetTargetDark(VR_TRG_Work *work)
{
	int flag,map;
	FVECTOR	core_size01[1] ;
	FVECTOR	off_set01[1] ;

	work->trg01_num = 1 ;
	work->trg02_num = 0 ;
	work->trg03_num = 0 ;

	work->core_trg01 = GV_Malloc( sizeof(TARGET)*work->trg01_num ) ;
	work->core_trg02 = NULL ;
	work->core_trg03 = NULL ;

	work->core_mdl = (1<<0) ;


/*LV1*/
	setfvec(&off_set01[0],0.0f,0.0f,0.0f);
	setfvec(&core_size01[0],TRG_DARK_SIZE,TRG_DARK_SIZE,TRG_DARK_SIZE);

	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);
	GM_CurrentMap =	map = work->control.map;
	MulFvec(&core_size01[0], VR_TARGET01_SCALE) ;
	GM_SetTarget(&work->b_trg,(flag|TARGET_CHILD_ALWAYS|TARGET_LOCKON),map,ENEMY_SIDE,&core_size01[0],&DG_ZeroVector);
	GM_SetTargetWeaponType( &work->b_trg, 0 ) ;
	GM_SetPowerTarget( &work->b_trg, &work->b_power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( &work->b_trg );
	MulFvec(&core_size01[0], 0.5f*VR_TARGET01_SCALE) ;
	MulFvec(&off_set01[0], VR_TARGET01_SCALE) ;
	GM_SetTarget( &work->core_trg01[0], flag,map, ENEMY_SIDE, &core_size01[0], &off_set01[0] ) ;
	GM_SetTargetParts( &work->b_trg, &work->core_trg01[0],1, 0 ) ;
	if(work->type & VR_TARGET_TYPE_NG){
		GM_SetTargetCallBack( &work->core_trg01[0], (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
		GM_SetTargetCallBack( &work->b_trg, (TARGET_CALLBACK)ChildTargCallBack_NG, work ) ;
	}else {
		GM_SetTargetCallBack(&work->core_trg01[0],(TARGET_CALLBACK) ChildTargCallBack_01, work ) ;
		GM_SetTargetCallBack( &work->b_trg,(TARGET_CALLBACK) ChildTargCallBack_Parent, work ) ;
	}
	GM_SetTargetWeaponType(&work->b_trg,0);
	GM_SetPowerTarget(&work->core_trg01[0],&work->b_power,POWER_DECREASE,500,0,0,&DG_ZeroVector );
#ifdef DEBUG_MODE
	if(work->type & VR_TARGET_DEBUG){
		NewTargetView( &work->core_trg01[0], 255, 0, 0 ) ;
	}
#endif
}

void init_vr_trgobj(VR_TRG_Work *work ,GV_ACT_EX	*actor){
	DG_DEF  *def ;
	int model,hlt_model ;

	work->p_act = actor ;

	work->status = 0 ;
	work->dmg_scale = 1.0f ;
	if(work->type & VR_TARGET_HIDESTART){
		work->status |= TRG_ST_HIDE ;
	}

	if ( GCL_GetOption( 'k' ) != NULL ) {
		model = GCL_GetNextInt() ;
	}

	/*基本部位的*/
	hlt_model = 0 ;
	work->hlt_rgb = VR_HLT_DEF ;
	work->core_rgb = VR_CORE_DEF ;
	work->ef_mdl_code = MDL_VR_TRG_TRI_RGB ;
	if(work->type & (VR_TARGET_TYPE_OCT)){
		model = MDL_VR_TRG_OCT_02 ;
		hlt_model = MDL_VR_TRG_OCT_HLT ;
		work->ef_mdl_code = MDL_VR_TRG_OCT_RGB ;
	}else if(work->type & VR_TARGET_TYPE_KATANA){
		model = MDL_VR_TRG_STR_02 ;
//		hlt_model = MDL_VR_TRG_STR_HLT ;
		hlt_model = 0 ;
		work->ef_mdl_code = MDL_VR_TRG_STR_RGB ;
	}else if(work->type & VR_TARGET_TYPE_CROSS){
		model = MDL_VR_TRG_SQU_01 ;
		hlt_model = MDL_VR_TRG_SQU_HLT ;
		work->ef_mdl_code = MDL_VR_TRG_SQU_RGB ;
	}else if(work->type & VR_TARGET_TYPE_MINE){
		model = MDL_VR_TRG_MINE ;
		hlt_model = 0 ;
		work->ef_mdl_code = 0  ;
	}else if(work->type & VR_TARGET_TYPE_HEX){
		model = MDL_VR_TRG_HEX ;
		hlt_model = 0 ;
		work->ef_mdl_code = MDL_VR_TRG_HEX_RGB ;
	}else if(work->type & VR_TARGET_TYPE_ONE){
		model = MDL_VR_TRG_CUBE_02 ;
		hlt_model = 0 ;
		work->ef_mdl_code = MDL_VR_TRG_CUB_RGB ;
	}else if(work->type & VR_TARGET_TYPE_ONE_TRI){
		model = MDL_VR_TRG_TRI_NOV ;
		hlt_model = 0 ;
		work->ef_mdl_code = MDL_VR_TRG_TRI_RGB ;
	} else if(work->type & (VR_TARGET_TYPE_DARK_OCTA)){
		model = MDL_VR_TRG_DARK_OCTA ;
		hlt_model = 0 ;
		work->ef_mdl_code = MDL_VR_TRG_OCT_RGB ;
	}else {
		model = MDL_VR_TRG_TRI_02 ;
		hlt_model = MDL_VR_TRG_TRI_HLT ;
		work->ef_mdl_code = MDL_VR_TRG_TRI_RGB ;
	}


	if(work->type & VR_TARGET_TYPE_NG){
		/*NG*/
		if(work->type & VR_TARGET_TYPE_OCT){
			model = MDL_VR_TRG_OCT_TAB_01 ;
		}else if(work->type & VR_TARGET_TYPE_KATANA){
			model = MDL_VR_TRG_STR_TAB_01 ;
		}else if(work->type & VR_TARGET_TYPE_HEX){
			model = MDL_VR_TRG_HEX_TAB ;
		}else if(work->type & VR_TARGET_TYPE_CROSS){
			model = MDL_VR_TRG_SQU_TAB ;
		}else if(work->type & VR_TARGET_TYPE_ONE){
			model = MDL_VR_TRG_CUBE_TAB ;
		}else if(work->type & VR_TARGET_TYPE_ONE_TRI){
			model = MDL_VR_TRG_TRI_NOV_TAB ;
		}else {
			model = MDL_VR_TRG_TRI_TAB_01 ;
		}

		work->hlt_rgb = VR_HLT_TAB ;
		work->core_rgb = VR_CORE_TAB ;
	}
	if(work->type & VR_TARGET_TYPE_BLAST){
		/*爆発的*/
		if(work->type & VR_TARGET_TYPE_OCT){
			model = MDL_VR_TRG_OCT_EXP_01 ;
		}else if(work->type & VR_TARGET_TYPE_KATANA){
			model = MDL_VR_TRG_STR_EXP_01 ;
		}else if(work->type & VR_TARGET_TYPE_MINE){
			model = MDL_VR_TRG_MINE ;
		}else if(work->type & VR_TARGET_TYPE_HEX){
			model = MDL_VR_TRG_HEX_EXP ;
		}else if(work->type & VR_TARGET_TYPE_CROSS){
			model = MDL_VR_TRG_SQU_EXP ;
		}else if(work->type & VR_TARGET_TYPE_ONE){
			model = MDL_VR_TRG_CUBE_EXP ;
		}else {
			model = MDL_VR_TRG_TRI_EXP_01 ;
		}
		work->hlt_rgb = VR_HLT_EXP ;
		work->core_rgb = VR_CORE_EXP ;
	}
	/*部位発光用モデル*/
	GM_InitObject(&(work->body),model,OBJECT_FLAG );
	GM_ConfigControlObject( &work->control, &work->body ) ;
	GM_ConfigObjectLight(&(work->body),work->lights) ;
	DG_GetPos(&work->efe_world) ;
	DG_PutObjs( work->body.objs );

//	DG_OBJS		hlt_objs ;

	if((hlt_model != 0)&&(!(work->type&VR_TARGET_STEALTH))){
		def = (DG_DEF*) GV_GetCache( GV_CacheID( hlt_model, 'k' ) ) ;
		work->hlt_objs = DG_MakeObjs( def, HLT_OBJECT_FLAG, DG_CHANL_MAIN ) ;
		DG_QueueObjs( work->hlt_objs ) ;
		DG_SetLightMatrix( work->hlt_objs, work->hlt_lights );
		work->hlt_lights[1].m[ 3 ][ 0 ] = (float) 127 ;
		work->hlt_lights[1].m[ 3 ][ 1 ] = (float) 127 ;
		work->hlt_lights[1].m[ 3 ][ 2 ] = (float) 127 ;
		vr_setflagall_objs(work->hlt_objs,DG_FLAG_INVISIBLE) ;
	}else {
		work->hlt_objs = NULL ;
	}

	work->mdl_code = model ;

	work->seek_num = -1 ;

	work->sight = 0 ;

	if((work->ef_mdl_code != 0)&&(!(work->type&VR_TARGET_NO_SIGHT))){
		float x,y,y_low ;
		x = work->body.objs->bound_max.vx ;
		if(x < fabs(work->body.objs->bound_min.vx)) x = fabs(work->body.objs->bound_min.vx) ;
		x *= 1.500f; 
		y = work->body.objs->bound_max.vy ;
		y_low = fabs(work->body.objs->bound_min.vy) ;


		if(work->type & (VR_TARGET_TYPE_OCT)){
			y += 300.0f;
			y_low += -150.0f;

		}else if(work->type & VR_TARGET_TYPE_KATANA){
			y += 300.0f;
			y_low += -150.0f;
		}else if(work->type & VR_TARGET_TYPE_CROSS){
			y += -61.0f;
			y_low += -200.0f;
		}else if(work->type & VR_TARGET_TYPE_HEX){
			y += -180.0f;
			y_low += -300.0f;
		}else if(work->type & VR_TARGET_TYPE_ONE){
			y += -61.0f;
			y_low += -200.0f;
		}else if(work->type & VR_TARGET_TYPE_ONE_TRI){
			y += 150.0f;
			y_low += 150.0f;
		}else if(work->type & (VR_TARGET_TYPE_DARK_OCTA)){
			y += 300.0f;
			y_low += -150.0f;
		}else {
			y += 150.0f;
			y_low += 150.0f;
		}


		GV_SetActorChild( actor, (void *)NewHomingSight( &work->sight,&work->body.objs->world,x,y,y_low)) ;
	}

	if(work->ef_mdl_code != 0 ){
		float shift ;
extern void *NewSIG_3DPOSPrint(FMATRIX * ,int ,int *,float ) ;
		shift = work->body.objs->bound_max.vy ;
//printf("shift[%f]\n",shift) ;

		if(work->type & (VR_TARGET_TYPE_OCT)){
			shift += 150.0f ;
		}else if(work->type & VR_TARGET_TYPE_KATANA){
//printf("KATANA shift[%f]\n",shift) ;
			shift += 150.0f ;
		}else if(work->type & VR_TARGET_TYPE_CROSS){
			shift += 181.0f ;
		}else if(work->type & VR_TARGET_TYPE_HEX){
			shift += 380.0f ;
		}else if(work->type & VR_TARGET_TYPE_ONE){
			shift += 281.0f ;
		}else if(work->type & VR_TARGET_TYPE_ONE_TRI){
		}
//printf("RESUKT shift[%f]\n",shift) ;
		if(!(work->type & VR_TARGET_TYPE_DARK_OCTA)){
			NewSIG_3DPOSPrint(&work->body.objs->world,VR_TypeFlagToNum(work->type),&work->body.objs->flag,shift) ;
		}
	}



	GM_InitRadarControl( &work->rctrl, (FVECTOR *) &work->body.objs->world.m[ 3 ], 0, -1 );
	GM_RadarSetVRange( &work->rctrl, RADAR_VRANGE_UPPER , RADAR_VRANGE_LOWER );

	work->build_sw =0 ;
	work->blast_cnt = 0 ;

	if( work->type & VR_TARGET_STEALTH) {
		extern void *NewOpticalCamouflage( DG_OBJS *objs, int flag );
		GV_SetActorChild( actor, (void *) NewOpticalCamouflage( work->body.objs, 0 ) ) ;
	}

	if(work->ef_mdl_code !=0){
		int cache ;
		OL_RANGE ol_range ;

		cache = GV_CacheID( work->ef_mdl_code, 'r' ) ;

		GV_SetActorChild( actor,(void *)NewObjectOutline( cache, work->body.objs , OL_FLAG_ERASETRANS ) );
		
		work->ol_col.col_num  = 0 ;
		work->ol_col.col[0].r = 127 ;
		work->ol_col.col[0].g = 127 ;
		work->ol_col.col[0].b = 127 ;
		work->ol_col.col[0].cd = 0 ;

		GV_CallChildSignalFunc( actor,OL_SIG_SetColor, (int) &work->ol_col ) ;
	}

	work->fl_pos_act = NULL ;
//	TRG_ST_HIDE			=	0x0020 ,

	work->scale = 1.0000000000000000f;
	work->scale_cnt = 0 ;
	if ( GCL_GetOption( 'v' ) != NULL ) {
		work->appear_cycle = DIRECT_TICK(GCL_GetNextInt()) ;
		work->scale_cycle = DIRECT_TICK(GCL_GetNextInt()) ;
	}else {
		work->scale_cycle = 0 ;
		work->appear_cycle = 0;
	}

	if(!(work->type & (VR_TARGET_TYPE_NG))){
		GM_SetHomingTrg(&work->hom_trg,&work->hom_mat,NULL,
			&(work->control.hzx_id),&work->control,0);
		GM_SetHomingTrgTarget( &work->hom_trg, &work->b_trg ) ;
		GM_PutHomingTrg(&work->hom_trg ) ;
	}
	work->str_work = NULL ;
	GV_SetActorChild( actor,work->str_work = (void *)NewSIG_3DPrintf(HIT_PRINT_SHIFT) );
	if ( GCL_GetOption( 'K' ) != NULL ) {
		work->katana_dir = GCL_GetNextInt() ;
	}else {
		work->katana_dir = -1 ;
	}
}
void free_vr_trgobj(VR_TRG_Work *work) {

	if(!(work->type & (VR_TARGET_TYPE_NG))){
		GM_FreeHomingTrg( &work->hom_trg ) ;
	}
	GM_FreeObject(&(work->body));
	if(work->hlt_objs != NULL ){
		DG_DequeueObjs( work->hlt_objs ) ;
		DG_FreeObjs( work->hlt_objs );
	}
    GM_FreeRadarControl( & work->rctrl ) ;
}
void VR_FreeMemory(VR_TRG_Work *work){
	if(work->trg01_num != 0) {
		GV_Free(work->core_trg01) ;
	}
	if(work->trg02_num != 0) {
		GV_Free(work->core_trg02) ;
	}
	if(work->trg03_num != 0) {
		GV_Free(work->core_trg03) ;
	}
	if(work->mine_trg != 0) {
		GV_Free(work->mine_trg) ;
	}
}
