/*

	fobj.h
	デザイナープレビュー環境用：人間プレビュー
	1999/07/07 S.Okajima
	2000/02/08 K.Kano Modified
	$Id: fobj.h,v 1.1.1.3 2002/11/19 11:43:27 Yoshizawa1 Exp $

*/


#ifndef _fobj_h_
#define _fobj_h_


#include "human.h"


typedef struct {
    CONTROL control;
    OBJECT body;
    FMATRIX lights[2];

    // DG_EVMOBJ *evm;

    FVECTOR pos;
    SVECTOR rot;

    int motion_index;
    int motion_max;

#if 1
    int rmt_anim_index;
    MTN_DATA mtn;
    int rmt_anim_frame;
#endif

    int disp_enable;

	/* ルートマトリクス(一番根っこのマトリクスを、
	   ワールドにするか、何かのHumanモデルの一部にするかを選択 */
	int root_mode;

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
    int change_rmt_anim_flag;

	/* 取り付ける人間のターゲット */
	int target_human_index;
	int human_object_index;

} FobjControl;

typedef struct {
    FobjControl *fobjs;
    int fobjs_size;
    int fobjs_index;

    int cursole_mode;
    int cursole_pos;
    int speed_mode;
} PreviewFobj_Param;


typedef struct {
	int model_strcode;
	int motion_index;
	int rmtanim_strcode;

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
} FobjSaveFormat;


#ifdef _WORK_DECLARED_
PreviewFobj_Param PreviewFobj;
#else
extern PreviewFobj_Param PreviewFobj;
#endif


void CopyFobjFromSavework(void);
void CopyFobjToSavework(void);


#if 0

void InitFobj( Work *work );
void ResetDataFobj( Work *work );

void Fobj_DebugPrint(Work *work);
void Fobj_DebugCursole(Work *work);
void Preview_FobjAct(Work *work);

#else

int InitFobj(void);
void ExitFobj(void);

void Preview_SetDefaultFobjData(void);

void FobjRestartAllMotion(void);

void Fobj_DebugPrint(void);
void Fobj_DebugCursole(void);
void Preview_FobjAct(void);
void *NewPreviewFobj(void);

void *NewKmsSkeletonTest(DG_OBJS *objs,int color);
void *NewEvmSkeletonTest(DG_EVMOBJ *obj,int color);

#endif


#endif
