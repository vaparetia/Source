#ifndef __MIP_MAP_H__
#define __MIP_MAP_H__

int GetPowLevel(int size);
int GetMipLevel(int size);
void GetPixelFromRECT2(RGBA *p_rgba,int x, int y, RECT2 *in1);
void MakeMipMapRECT2RECT(RECT2 *in1, RECT2 *in2, int level);
void MakeMipMap(RECT2 *prect,int mip_level,RECT2 *miprect_pointer[],int *num,int format);

#endif
