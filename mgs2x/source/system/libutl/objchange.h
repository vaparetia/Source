/*
	objchange.h
		モデルの部分変更

	1999/12/07 K.Kano
	$Id: objchange.h,v 1.1.1.3 2002/11/19 11:42:57 Yoshizawa1 Exp $
*/


#ifndef _objchange_h_
#define _objchange_h_


typedef struct {
    DG_OBJS *objs;

    /* モデルを構成するパーツ */
    DG_DEF *def;
    DG_DEF *parts;

    CV2_DEF *def_cv2;
    CV2_DEF *parts_cv2;

    /* 現在使用中のパーツをファイルフォーマットの形式に直したもの */
    DG_DEF *nowdef;
    CV2_DEF *nowcv2;

    short *mdl_list_index;
} OBJCHANGE_WORK;


typedef struct {
    OBJCHANGE_WORK *objch;
    int type;
} OBJECT_CHG;


OBJCHANGE_WORK *GM_InitObjectForObjChange(OBJECT *object,int model,int parts_model,
					  int *mdl_list,int flag);
OBJCHANGE_WORK *GM_InitObject2ForObjChange(OBJECT *object,DG_DEF *def,int flag);
void GM_FreeObjectForObjChange(OBJECT *object,OBJCHANGE_WORK *work);

void ChangeObj(OBJCHANGE_WORK *work,int objnum,int partsnum);
void ChangeObjs(OBJCHANGE_WORK *work,int *mdl_list);
void ChangeObjByMDL(OBJCHANGE_WORK *work,int objnum,DG_MDL *mdl);
void ChangeObjsByMDL(OBJCHANGE_WORK *work,DG_MDL **mdl_list);


/* OBJ CHANGEを利用する場面で、使われる可能性が高いと思われるので、
   標準として宣言しておく */
enum {
    OBJCH_TYPE_A=0,
    OBJCH_TYPE_B,
    OBJCH_TYPE_C,
    OBJCH_TYPE_D,

    OBJCH_TYPE_E,
    OBJCH_TYPE_F,
    OBJCH_TYPE_G,
    OBJCH_TYPE_H,

    OBJCH_TYPE_I,
    OBJCH_TYPE_J,
    OBJCH_TYPE_K,
    OBJCH_TYPE_L,

    OBJCH_TYPE_M,
    OBJCH_TYPE_N,
    OBJCH_TYPE_O,
    OBJCH_TYPE_P,

    OBJCH_TYPE_Q,
    OBJCH_TYPE_R,
    OBJCH_TYPE_S,
    OBJCH_TYPE_T,

    OBJCH_TYPE_U,
    OBJCH_TYPE_V,
    OBJCH_TYPE_W,
    OBJCH_TYPE_X,

    OBJCH_TYPE_Y,
    OBJCH_TYPE_Z,


    OBJCH_TYPE_00=0,
    OBJCH_TYPE_01,
    OBJCH_TYPE_02,
    OBJCH_TYPE_03,

    OBJCH_TYPE_04,
    OBJCH_TYPE_05,
    OBJCH_TYPE_06,
    OBJCH_TYPE_07,

    OBJCH_TYPE_08,
    OBJCH_TYPE_09,
    OBJCH_TYPE_10,
    OBJCH_TYPE_11,

    OBJCH_TYPE_12,
    OBJCH_TYPE_13,
    OBJCH_TYPE_14,
    OBJCH_TYPE_15,

    OBJCH_TYPE_16,
    OBJCH_TYPE_17,
    OBJCH_TYPE_18,
    OBJCH_TYPE_19,

    OBJCH_TYPE_20,
    OBJCH_TYPE_21,
    OBJCH_TYPE_22,
    OBJCH_TYPE_23,

    OBJCH_TYPE_24,
    OBJCH_TYPE_25,
    OBJCH_TYPE_26,
    OBJCH_TYPE_27,

    OBJCH_TYPE_28,
    OBJCH_TYPE_29,
    OBJCH_TYPE_30,
    OBJCH_TYPE_31,

    OBJCH_TYPE_32,
    OBJCH_TYPE_33,
    OBJCH_TYPE_34,
    OBJCH_TYPE_35,

    OBJCH_TYPE_36,
    OBJCH_TYPE_37,
    OBJCH_TYPE_38,
    OBJCH_TYPE_39,
};


#endif
