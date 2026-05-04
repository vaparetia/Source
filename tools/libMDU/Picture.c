/*
   Picture.c

   P3DPICTURE内のα値を演算

   by M.Sonoyama 1999.Sep.～ 
   Remade by K.Kano , 3/1/2000

   $Id: Picture.c,v 1.1 2002/09/13 09:02:21 usr01363 Exp $

   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <SFDLINUX.h>

#include "METALGEAR.h"
#include "fmt_km4.h"
#include "MDU_util.h"
#include "MDU_tex.h"


/* PICTUREに含まれるα値の平均を求める */
float GetAlphaAverageFromPicture(HP3DPICTURE hp3dpic)
{
	LPBITMAPINFOHEADER info;
	int i,j;
	float sum=0.0f;
	int npoints;
	int AlphaWidth;


	info=(LPBITMAPINFOHEADER)(P3DPicture(hp3dpic)->hDIB);

	/* αプレーンの横幅、4byte単位 */
	AlphaWidth=(info->biWidth*8+31)/32*4;

	/* 総ピクセル数 */
	npoints=info->biWidth*info->biHeight;

	/* α値の総合計を求める */
	for(j=0;j<info->biHeight;j++){
		LPBYTE alphap;

		alphap=(LPBYTE)info+sizeof(BITMAPINFOHEADER)+GetBMPPaletteSize(info)
			+info->biSizeImage+AlphaWidth*j;

		for(i=0;i<info->biWidth;i++){
			sum+=(float)*alphap;
			alphap++;
		}
	}

	// printf("%s %f %d\n",P3DPicture(hp3dpic)->szName,sum,npoints);

	/* 平均化 */
	return sum/(float)npoints;
}
