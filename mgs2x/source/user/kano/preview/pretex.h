/*
	pretex.c
		プレビュー用テクスチャ管理

	2000/2/12 K.Kano
	$Id: pretex.h,v 1.1.1.3 2002/11/19 11:43:31 Yoshizawa1 Exp $
*/


#ifndef _pretex_h_
#define _pretex_h_


typedef struct {
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	int			tex_width;
	int			tex_height;

	int			mode;
    int			tex_list_num1;
    int			tex_list_num2;
} PreviewTex_Param;


#ifdef _WORK_DECLARED_
PreviewTex_Param PreviewTex;
#else
extern PreviewTex_Param PreviewTex;
#endif


void Pretex_InitPacket( int witch );
void Tex_DebugPrint(void);
void Tex_DebugCursole(void);
void Preview_SetTexDefaultData(void);
int Preview_InitTex(void);
void Preview_ExitTex(void);


#endif
