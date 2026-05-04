#ifndef __TEX_UTL_H__
#define __TEX_UTL_H__

MDU_Tex *LoadTex(char *name);
void MakeFullColorTex(MDU_Tex *tex);
u_char *MakeFullColorBmp(MDU_Tex *tex, int depth);
//int *MakeFullColorBmp(MDU_Tex *tex, int depth);
void MakeDDSfromIMG(char *out,int w,int h,void *pimg,int mode);

#endif