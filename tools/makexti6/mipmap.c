//------------------------------
//
//  mipmap.c
//
//	NISINO Motoaki
//
//------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "my.h"

#include "MDU_util.h"
#include "MDU_tex.h"

#include "makexti.h"

#include "bmp.h"
#include "arrangerect.h"
#include "file.h"
#include "fmt_cm2x.h"
#include "dds.h"
#include "dxtcmp.h"
#include "wrapfunc.h"
#include "mipmap.h"

int GetPowLevel(int size)
{
	int i=1;
	while(i<size){
		i*=2;
	}
	return(i);
}

//..ミップマップがいくらの深さまで作れるかを得る
int GetMipLevel(int size)
{
	int i=0;
	
	while(size > 4){//..4pixelまで
		size/=2;
		i++;
	}
	return(i);
}

//..32bit限定
void GetPixelFromRECT2(RGBA *p_rgba,int x, int y, RECT2 *in1)
{
	RGBA *p;
	
	ASSERT(in1->byte == 4);
	ASSERT(x < in1->w);
	ASSERT(y < in1->h);
	ASSERT(x >= 0);
	ASSERT(y >= 0);
	
	p =(RGBA*)(in1->pimg + in1->pitch * y + x * 4);

	p_rgba->r= p->r;
	p_rgba->g= p->g;
	p_rgba->b= p->b;
	p_rgba->a= p->a;

}




/*
入力矩形からミップマップを作成
in1		出力矩形
in2		入力矩形
level	ミップマップのレベル。何段階のものを作成するか
		1-->1/2  2-->1/4  3-->1/8
*/

void MakeMipMapRECT2RECT(RECT2 *in1, RECT2 *in2, int level)
{
	int i,j,hohaba,p1_x,p1_y,p2_x,p2_y;
	unsigned int r_sum=0,g_sum=0,b_sum=0,a_sum=0;
	//char *p1,*p2;
	RGBA *rgba1,*rgba2,pixel;
	rgba1=(RGBA *)in1->pimg;
	rgba2=(RGBA *)in2->pimg;

	hohaba = pow(2,level) ;
		
	for( p1_y=0; p1_y < in1->h; p1_y++){
		rgba1 = (RGBA *)(in1->pimg + in1->pitch * p1_y);
		for( p1_x=0; p1_x < in1->w; p1_x++, rgba1++){
		
			p2_x=p1_x * hohaba;
			p2_y=p1_y * hohaba;
			
			//..1pixelの平均を計算
			r_sum = g_sum = b_sum = a_sum = 0;
			for( j=0; j < hohaba; j++ ){
				for( i=0; i < hohaba  ; i++){
					//..矩形in2の範囲外を指定していないかチェック
					if(p2_x+i >= in2->w ||  p2_y+j >= in2->h) break;
					
					GetPixelFromRECT2(&pixel, p2_x+i, p2_y+j, in2);
					r_sum += pixel.r;
					g_sum += pixel.g;
					b_sum += pixel.b;
					a_sum += pixel.a;
					
				}
			}
			rgba1->r = r_sum/(hohaba*hohaba);
			rgba1->g = g_sum/(hohaba*hohaba);
			rgba1->b = b_sum/(hohaba*hohaba);
			rgba1->a = a_sum/(hohaba*hohaba);
			
		}
	}
}




/*
	ミップマップ作成関数

	prect		もとのテクスチャ
	mip_level	ミップマップのレベル。何段階のものを作成するか
				0-->all  2-->1/2  3-->1/4  4-->1/8
	pimg		作成されたミップマップが置かれるアドレス
	num			何個作成されたかを覚えておく変数
	
注意：現在のバージョンでは最小のミップマップは4*4になります。

	
*/
void MakeMipMap(RECT2 *prect,int mip_level,RECT2 *miprect_pointer[],int *num,int format)
{
	int w,h,hohaba,i,max_mip_num;
	//RECT2 miprect;
	RECT2 *pmip;

	int	compress_size ;
	void *pcmpimg;

	//..矩形を2の乗数倍にそろえる
	w=GetPowLevel(prect->w);
	h=GetPowLevel(prect->h);
	
	max_mip_num = GetMipLevel(w) < GetMipLevel(h) ? GetMipLevel(w) : GetMipLevel(h);
	
	if(mip_level>0){
		if(mip_level > max_mip_num) mip_level=max_mip_num;
	
		pmip=(RECT2*)Malloczero( sizeof(RECT2)*mip_level );
		*num=mip_level;
	}else{
		//..最小まで作る
		
		mip_level = max_mip_num;
		pmip=(RECT2 *)Malloc( sizeof(RECT2)*mip_level );
		*num=mip_level;
	}
	//dprint(mip_level);
	
	dprint(mip_level);
	for(i=1; i<mip_level; i++,pmip++){
		//char mipmapfilename[128];
		//..ミップマップ用ワークを確保
		dprint(i);
		hohaba = pow(2,i);
		//dprint(hohaba);
		ASSERT(w/hohaba>0);
		ASSERT(h/hohaba>0);
		
		RECT2Create(pmip, w/hohaba, h/hohaba, 4);
		
		//..ミップマップ作成
		MakeMipMapRECT2RECT(pmip, prect, i);
		
		miprect_pointer[i-1]=pmip;
		
		
		//..圧縮するかい？
		if(format==TEXPACK_FORMAT_A8R8G8B8)
		{
			//printf("A8R8G8B8\n");
		}
		else if(format==TEXPACK_FORMAT_DXT1)
		{
			dprint(pmip->w);dprint(pmip->h);
			
			//..4*4pixel未満のテクスチャは圧縮できない
			if(pmip->w < 4 || pmip->h < 4){
				ASSERT(pmip->w > 4);
				ASSERT(pmip->h > 4);
				continue;
			}
			
			CompressImageDXT1((unsigned int *)pmip->pimg, pmip->w, pmip->h, &pcmpimg, &compress_size );
			miprect_pointer[i-1]->pimg = pcmpimg;
			miprect_pointer[i-1]->buffer_size = compress_size;

#if 0			
			//..ファイル出力
			sprintf(mipmapfilename,"mipdds_%d.dds",i);
			MakeDDSfromIMG(mipmapfilename, pmip->w, pmip->h, pcmpimg,D3DFMT_DXT1);
#endif
		
		}
		else if(format==TEXPACK_FORMAT_DXT3)
		{
			
			//printf("DXT3\n");
			
			dprint(pmip->w);dprint(pmip->h);
			
			
			//..4*4pixel未満のテクスチャは圧縮できない
			if(pmip->w < 4 || pmip->h < 4){
				ASSERT(pmip->w > 4);
				ASSERT(pmip->h > 4);
				continue;
			}
			
			CompressImageDXT3((unsigned int *)pmip->pimg, pmip->w, pmip->h, &pcmpimg, &compress_size );
			miprect_pointer[i-1]->pimg = pcmpimg;
			miprect_pointer[i-1]->buffer_size = compress_size;
#if 0			
			//..ファイル出力
			sprintf(mipmapfilename,"mipdds_%d.dds",i);
			MakeDDSfromIMG(mipmapfilename, pmip->w, pmip->h, pcmpimg,D3DFMT_DXT3);
#endif
		}
		else if(format==TEXPACK_FORMAT_DXT5)
		{
			
			//printf("DXT3\n");
			
			dprint(pmip->w);dprint(pmip->h);
			
			
			//..4*4pixel未満のテクスチャは圧縮できない
			if(pmip->w < 4 || pmip->h < 4){
				ASSERT(pmip->w > 4);
				ASSERT(pmip->h > 4);
				continue;
			}
			
			CompressImageDXT5((unsigned int *)pmip->pimg, pmip->w, pmip->h, &pcmpimg, &compress_size );
			miprect_pointer[i-1]->pimg = pcmpimg;
			miprect_pointer[i-1]->buffer_size = compress_size;
#if 0			
			//..ファイル出力
			sprintf(mipmapfilename,"mipdds_%d.dds",i);
			MakeDDSfromIMG(mipmapfilename, pmip->w, pmip->h, pcmpimg,D3DFMT_DXT3);
#endif
		}
	}
}











