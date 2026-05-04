/*

	object.h
	デザイナープレビュー環境用：オブジェクトプレビュー
	1999/07/07 S.Okajima
	2000/02/09 K.Kano Modified
	$Id: object.h,v 1.1.1.3 2002/11/19 11:43:29 Yoshizawa1 Exp $

*/


#ifndef _object_h_
#define _object_h_


#define	FLAG_PRE_SHADE		(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_ONEPIECE| \
							 DG_FLAG_PAINT|DG_FLAG_FINISHCALC)
#define	FLAG_NORMAL_SHADE	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_ONEPIECE| \
							 DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


typedef struct {
    DG_DEF  *def;
    DG_OBJS *objs;
    FMATRIX lights[2];

    FVECTOR pos;
    SVECTOR rot;

    int disp_enable;
    int shade_mode;

    int target_human_index;
    int human_object_index;
} ObjectControl;

typedef struct {
    ObjectControl *objects;
    int objects_size;
    int objects_index;

    int cursole_mode;
    int cursole_pos;
    int speed_mode;
} PreviewObject_Param;


typedef struct {
	int model_strcode;

    FVECTOR pos;
    SVECTOR rot;

    int disp_enable;
    int shade_mode;

	int target_human_strcode;
    int human_object_index;
} ObjectSaveFormat;


#ifdef _WORK_DECLARED_
PreviewObject_Param PreviewObject;
#else
extern PreviewObject_Param PreviewObject;
#endif



void CopyObjectToSavework(void);
void CopyObjectFromSavework(void);


#if 0

void InitObjects( Work *work );
void ResetDataObjects( Work *work );

void Object_DebugPrint(Work *work);
void Object_DebugCursole(Work *work);
void Preview_ObjectAct(Work *work);

#else

int InitObjects(void);
void Objects_SetDefaultData(void);
void ResetDataObjects(void);

void Object_DebugPrint(void);
void Object_DebugCursole(void);
void Preview_ObjectAct(void);
void *NewPreviewObject(void);

#endif


#endif
