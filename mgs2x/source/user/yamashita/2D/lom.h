/*******************************************************************************
 * layout manager - lom.h
 * レイアウトマネージャ
 * 2002/04/16 S.Yamashita
 * $Id: lom.h,v 1.1.1.3 2002/11/19 11:51:39 Yoshizawa1 Exp $
 */

#ifndef __INC_LOM__
#define __INC_LOM__

/*******************************************************************************
 * include
 */

#include "layoutman.h"

/*******************************************************************************
 * definitions and typedefs and structures
 */

#define __CHARA_NAME__ "Layout Manager"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * functions
 */

/* lom_get.c */
int         LOM_GetActlistIndex(LAYOUTMAN   *layoutman  , int lom_actlist_name);
LOM_ACTLIST *LOM_GetActlist    (LAYOUTMAN   *layoutman  , int lom_actlist_name);
LOM_ACTLIST *LOM_GetActlistI   (LAYOUTMAN   *layoutman  , int lom_actlist_index);
int         LOM_GetActIndex    (LOM_ACTLIST *lom_actlist, int lom_act_name);
LOM_ACT     *LOM_GetAct        (LOM_ACTLIST *lom_actlist, int lom_act_name);
LOM_ACT     *LOM_GetAct2       (LAYOUTMAN   *layoutman  , int lom_actlist_name, int lom_act_name);
LOM_ACT     *LOM_GetActI       (LOM_ACTLIST *lom_actlist, int lom_act_index);
LOM_ACT     *LOM_GetActI2      (LAYOUTMAN   *layoutman  , int lom_actlist_index, int lom_act_index);

int         LOM_GetIcolistIndex(LAYOUTMAN   *layoutman  , int lom_icolist_name);
LOM_ICOLIST *LOM_GetIcolist    (LAYOUTMAN   *layoutman  , int lom_icolist_name);
LOM_ICOLIST *LOM_GetIcolistI   (LAYOUTMAN   *layoutman  , int lom_icolist_index);
int         LOM_GetIcoIndex    (LOM_ICOLIST *lom_icolist, int lom_ico_name);
LOM_ICO     *LOM_GetIco        (LOM_ICOLIST *lom_icolist, int lom_ico_name);
LOM_ICO     *LOM_GetIco2       (LAYOUTMAN   *layoutman  , int lom_icolist_name, int lom_ico_name);
LOM_ICO     *LOM_GetIcoI       (LOM_ICOLIST *lom_icolist, int lom_ico_index);
LOM_ICO     *LOM_GetIcoI2      (LAYOUTMAN   *layoutman  , int lom_icolist_index, int lom_ico_index);

int         LOM_GetObjlistIndex(LAYOUTMAN   *layoutman  , int lom_objlist_name);
LOM_OBJLIST *LOM_GetObjlist    (LAYOUTMAN   *layoutman  , int lom_objlist_name);
LOM_OBJLIST *LOM_GetObjlistI   (LAYOUTMAN   *layoutman  , int lom_objlist_index);
int         LOM_GetObjIndex    (LOM_OBJLIST *lom_objlist, int lom_obj_name);
LOM_OBJ     *LOM_GetObj        (LOM_OBJLIST *lom_objlist, int lom_obj_name);
LOM_OBJ     *LOM_GetObj2       (LAYOUTMAN   *layoutman  , int lom_objlist_name, int lom_obj_name);
LOM_OBJ     *LOM_GetObjI       (LOM_OBJLIST *lom_objlist, int lom_obj_index);
LOM_OBJ     *LOM_GetObjI2      (LAYOUTMAN   *layoutman  , int lom_objlist_index, int lom_obj_index);

int         LOM_GetPadlistIndex(LAYOUTMAN   *layoutman  , int lom_padlist_name);
LOM_PADLIST *LOM_GetPadlist    (LAYOUTMAN   *layoutman  , int lom_padlist_name);
LOM_PADLIST *LOM_GetPadlistI   (LAYOUTMAN   *layoutman  , int lom_padlist_index);
LOM_PAD     *LOM_GetPadI       (LOM_PADLIST *lom_padlist, int lom_pad_index);
LOM_PAD     *LOM_GetPadI2      (LAYOUTMAN   *layoutman  , int lom_padlist_index, int lom_pad_index);

int         LOM_GetModeIndex   (LAYOUTMAN   *layoutman  , int lom_mode_name);
LOM_MODE    *LOM_GetMode       (LAYOUTMAN   *layoutman  , int lom_mode_name);
LOM_MODE    *LOM_GetModeI      (LAYOUTMAN   *layoutman  , int lom_mode_index);

/* lom_new.c */
int LOM_NewActlist(LAYOUTMAN *layoutman, int actlist_name, int n_lom_act);
int LOM_NewIcolist(LAYOUTMAN *layoutman, int icolist_name, int n_lom_ico);
int LOM_NewObjlist(LAYOUTMAN *layoutman, int objlist_name, int n_lom_obj);
int LOM_NewPadlist(LAYOUTMAN *layoutman, int padlist_name, int n_lom_pad);
int LOM_NewMode   (LAYOUTMAN *layoutman, int mode_name, int lom_actlist_index, int lom_objlist_index, int lom_padlist_index);
int LOM_NewMode2  (LAYOUTMAN *layoutman, int mode_name, int lom_actlist_name, int lom_objlist_name, int lom_padlist_name);

/* lom_load.c */
int LOM_LoadActlist(LAYOUTMAN *layoutman, int *data, int *pos);
int LOM_LoadIcolist(LAYOUTMAN *layoutman, int *data, int *pos);
int LOM_LoadObjlist(LAYOUTMAN *layoutman, int *data, int *pos);
int LOM_LoadPadlist(LAYOUTMAN *layoutman, int *data, int *pos);
int LOM_LoadMode   (LAYOUTMAN *layoutman, int *data, int *pos);

/*******************************************************************************
 */

#endif	/* __INC_LOM__ */
