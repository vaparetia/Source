/*
	va_preview.h
		頂点アニメのプレビュー

	1999/12/13 K.Kano
	$Id: va_preview.h,v 1.1.1.3 2002/11/19 11:43:32 Yoshizawa1 Exp $
*/


#ifndef _va_preview_h_
#define _va_preview_h_


#define MAX_VA_PATTERN			32


typedef struct {
    void *va;
    int va_size;
    int va_index;

    char *va_filename[MAX_VA_PATTERN];
    int	 va_filename_size;
} PreviewVA_Param;


#ifdef _WORK_DECLARED_
PreviewVA_Param PreviewVA;
#else
extern PreviewVA_Param PreviewVA;
#endif


void VAPreview_DebugPrint(void);
void VAPreview_DebugCursole(void);
void *NewVAPreview_called(FVECTOR *x,SVECTOR *r);


#endif
