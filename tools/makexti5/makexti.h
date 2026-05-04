#ifndef __MAKEXTI_H__
#define __MAKEXTI_H__

MDU_Tex *LoadTex(char *name);
void MakeFullColorTex(MDU_Tex *tex);
u_char *MakeFullColorBmp(MDU_Tex *tex, int depth);
//int *MakeFullColorBmp(MDU_Tex *tex, int depth);


#endif