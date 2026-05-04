/*
	trg_cmd.h
	的破壊ステージだけで使いそうなものはVR_SYSからこちらへ移動。メモリ節約のため
	2002/04/03 K.Sigeno
	$Id: trg_cmd.h,v 1.1.1.3 2002/11/19 11:49:56 Yoshizawa1 Exp $
*/

#include "../../yamashita/outline/outline.h"

void *NewObjectOutline(int cache,DG_OBJS *dg_objs,int flag) ;

//#define TRG_DARK_SIZE (800.0f)
#define TRG_DARK_SIZE (700.0f)

#define TRG_DARK_SCALE_MIN (0.2f)

typedef	struct _VR_TRG_Work {
	HOMING_TRG		hom_trg ;
	FMATRIX			lights[2] ;
	FMATRIX			hlt_lights[2] ;
	FMATRIX			efe_world ;
	FMATRIX			hom_mat ;

	FVECTOR			hom_shift ;
	FVECTOR			fl_pos ;
	OBJECT			body;
	CONTROL			control;
	RADAR_CTRL		rctrl ;
	TARGET			b_trg;
	POWER_TARGET	b_power;
	OL_COLOR		ol_col ;
	CVECTOR			hlt_rgb;
	CVECTOR			core_rgb;

	long64			last_weapon_type ;	/*最後に受けた攻撃タイプ*/

	DG_OBJS			*hlt_objs ;
	TARGET			*mine_trg ;

	TARGET			*core_trg01;
	TARGET			*core_trg02;
	TARGET			*core_trg03;
	void*			fl_pos_act ;
	void*			p_act ;
	void*			str_work ;
	void*			mine_act ;

	int				type ;
	int				test_flag ;
	int				name ;
	int				proc_id ;
	int				cnt ;
	int				mdl_code;
	int				ef_mdl_code;
	int				sight ;
	int				build_sw ;

	float			jiten ;
	float			scale ;
	float			punch_force ;
	float			dmg_scale ;

	short			katana_dir ;
	short			visi_cnt ;
	short			bound_cnt ;
	short			vital ;

	short			core_mdl ;
	short			scale_cnt;
	short			scale_cycle;
	short			appear_cycle;

	short			finish ;	/*とどめをさした時の得点 ボーナス加算に使用*/
	short			mode ;
	short			mode_buf ;
	short			status ;

	short			dir_off ; /*自転初期*/

	char			seek_num ;	/*シーク中オブジェ番号*/
	char			trg01_num ;

	char			trg02_num ;
	char			trg03_num ;

	char			blast_cnt ;
	char			amb_cnt ;

} VR_TRG_Work ;


extern	int		VR_TypeFlagToNum(int) ;
extern	int		VR_TrgHitFunc(void *,FVECTOR	*,FVECTOR	*,FVECTOR *,int ,int) ;
extern	void VR_SetBlastRange(void) ;
extern	void VR_SetTrgScale(void) ;
extern	void NewVRTarget1life(void) ;
extern	void NewVRWallLife(void) ;
extern	void NewVRCubeLife(void) ;
extern	void VR_SetTargetSpeedRate(void) ;
extern	void VR_SetComboLimit(void) ;
extern	void VR_SetComboCnt(void) ;
extern	void VR_RestComboCnt(void) ;
extern	u_char VR_GetPartsDmg(int ,int) ;
extern	void VR_SetTargetScore(void) ;
extern	void VR_SetTargetStrength(void) ;
extern	void CallBreakObj(int ,FMATRIX *) ;
extern	void VR_TRG_Clash_SE(FVECTOR *,int) ;
extern	void VR_DestoryComboCheck(int,FVECTOR *) ;
extern	void VR_ResetCombo(FVECTOR *) ;

extern void *NewSIG_3DPrintf(int) ;
extern void CALL_SIG_3DPrintf(void *,FVECTOR * ,char *,int ,int ) ;

