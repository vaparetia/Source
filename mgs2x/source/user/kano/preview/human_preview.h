/*
	human_preview.h
		モデルすり替えのプレビュー用人形

	1999/12/10 K.Kano
	$Id: human_preview.h,v 1.1.1.3 2002/11/19 11:43:28 Yoshizawa1 Exp $
*/


#ifndef _human_preview_h_
#define _human_preview_h_


#define MAX_OBJCHANGE_PATTERN		64

typedef struct {
    void *objchange;
    int objchange_size;
    int objchange_index;

    DG_DEF **objchange_def;
    int	objchange_def_size;
} PreviewHumanOC_Param;


#ifdef _WORK_DECLARED_
PreviewHumanOC_Param PreviewHumanOC;
#else
extern PreviewHumanOC_Param PreviewHumanOC;
#endif


void HumanOC_DebugPrint(void *work,int index);
void HumanOC_DebugCursole(void *work);
void *NewOCHumanPreview_called(FVECTOR *x,SVECTOR *r,int mname,int mname2,int mtname);

void HumanOC2_DebugPrint();
void HumanOC2_DebugCursole();
void *NewOCHumanPreview2_called(FVECTOR *x,SVECTOR *r,int mtname);


#endif
