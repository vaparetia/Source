/*

	human.h
	デザイナープレビュー環境用：人間プレビュー
	1999/07/07 S.Okajima
	2000/02/08 K.Kano Modified
	$Id: human.h,v 1.1.1.3 2002/11/19 11:43:27 Yoshizawa1 Exp $

*/


#ifndef _human_h_
#define _human_h_

typedef struct {
	int		length ;
	int		n_joints ;
	FVECTOR		*move ;
	FVECTOR		*rots ;
	FVECTOR		*trans ;
} MTN_DATA ;


typedef struct {
    CONTROL control;
    OBJECT body;
    FMATRIX lights[2];

    // DG_EVMOBJ *evm;

    FVECTOR pos;
    SVECTOR rot;

    int mar_index;
    int motion_index;
    int motion_max;

    int face_anim_index;
    MTN_DATA mtn;
    int face_anim_frame;
	int mtn_move_enable;

    int disp_enable;

    /* 動いているか止まっているかのフラグ */
    int motion_mode;

    /* 重力の有効／無効フラグ */
    int gravity_flag;

    /* バウンディングボックスの表示/非表示 */
    int boundtest_enable;

    int pos_fix_flag;
    int interp;

	int skeltest_enable;

	int shadow_enable;

    /* カーソル等の操作によって、モーション等を変更した場合に立つ */
    int change_motion_flag;
    int change_face_anim_flag;

} HumanControl;

typedef struct {
    HumanControl *humans;
    int humans_size;
    int humans_index;

    int cursole_mode;
    int cursole_pos;
    int speed_mode;
} PreviewHuman_Param;


typedef struct {
	int model_strcode;
	int motion_strcode;
	int motion_index;
	int faceanim_strcode;

    FVECTOR pos;
    SVECTOR rot;

    int disp_enable;

    /* 動いているか止まっているかのフラグ */
    int motion_mode;

    /* 重力の有効／無効フラグ */
    int gravity_flag;

    /* バウンディングボックスの表示/非表示 */
    int boundtest_enable;

    int pos_fix_flag;
    int interp;

	int skeltest_enable;
} HumanSaveFormat;


#ifdef _WORK_DECLARED_
PreviewHuman_Param PreviewHuman;
#else
extern PreviewHuman_Param PreviewHuman;
#endif


void CopyHumanToSavework(void);
void CopyHumanFromSavework(void);


#if 0

void InitHuman( Work *work );
void ResetDataHuman( Work *work );

void Human_DebugPrint(Work *work);
void Human_DebugCursole(Work *work);
void Preview_HumanAct(Work *work);

#else

int InitHuman(void);
void ExitHuman(void);

void Preview_SetDefaultHumanData(void);

void SetMtnMotionData( MTN_DATA *mtn_data, int name_id );
void EvmActMotion(DG_EVMOBJ *evmobj,int first,int n_joints,
				  FVECTOR *skel_trans,FVECTOR *skel_rot,DG_OBJS *objs);
void EvmActMotionDummy(DG_EVMOBJ *evmobj,int first,FMATRIX *mat);

void HumanRestartAllMotion(void);

void Human_DebugPrint(void);
void Human_DebugCursole(void);
void Preview_HumanAct(void);
void *NewPreviewHuman(void);

void *NewKmsSkeletonTest(DG_OBJS *objs,int color);
void *NewEvmSkeletonTest(DG_EVMOBJ *obj,int color);

#endif


#endif
