//------------------------------
//
//  MGS2X  Texture Postion Arrangemet & Packing Tool
//
//		makexti6
//
//		2001/04/25
//		2002/04/10	Xbox新形式に対応
//		NISINO Motoaki (X)
//
//------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my.h"

#include "MDU_util.h"
#include "MDU_tex.h"

#include "makexti.h"

#include "bmp.h"

#include "file.h"
#include "fmt_cm2x.h"
#include "dds.h"
#include "tex_utl.h"
#include "wrapfunc.h"
#include "arrangerect.h"
#include "dxtcmp.h"
#include "mipmap.h"


int num_tex=0;
int flg_test_tex=0;
char *format;
int format_type=0;
int limit;
int min;
int num_xti=0;
int dds_flg=0;
int mipmap_flg=0;
int miplevel=0;
int mipmap_num=0;
RECT2 *mipmap_rect[20];

int Usage(void){
	printf("Usage : \n");
	printf("\tmakexti6 [-i (inputfile)] [-o (outputfile)] [-f (format_)] \n");
	printf("\t-i : inputfile (*.bmp *.pic)\n");
	printf("\t-o : output basename. (tpk)\n");
	printf("\t-f : texture format.  Select BMP32 or DXT3.\n");
	printf("\t-m : tpk minmum size. defult 32pixel. 2^x\n");
	printf("\t-l : tpk max size. defult 4096pixel. 2^x\n");
	printf("\t-d : Output DDS file.\n");
	printf("\t-t : Make Mipmap texture. [ 0-->all  1-->1/2  2-->1/4]\n");
	exit(1);
}



int main(int argc,char **argv)
{
	void *buff;
	void *p_header_img,*p_tex_img;
	int header_size,tex_size;
	int i;
	char *in,*out;
	char outfile[ 512 ];
	RECT2 rect_all;
	RGBA	clut_data[256] ;


	MDU_ParseOption( argc, argv );

    if (MDU_GetOption('o') == NULL) {
		Usage();
	}
	out = MDU_GetNextValue();
	strcpy(outfile,out);
	
	//..フォーマットオプション
    if (MDU_GetOption('f') != NULL) {
		format=MDU_GetNextValue();
	}else{
		format=NULL;
	}

	//..矩形の大きさ下限
	if(MDU_GetOption('m') != NULL ){
		char *p;
		p=MDU_GetNextValue();
		min=atoi(p);
	}else{
		min = 32;
	}
	dprint(min);
	
	//..矩形の大きさ上限
	if(MDU_GetOption('l') != NULL ){
		char *p;
		p=MDU_GetNextValue();
		limit=atoi(p);
	}else{
		limit = 4096;
	}
	dprint(limit);

	//..DDSファイルを出力
	if(MDU_GetOption('d') != NULL ){
		dds_flg=1;
	}

	//..mipmap出力
	if(MDU_GetOption('t') != NULL ){
		char *p;
		mipmap_flg=1;
		p=MDU_GetNextValue();
		miplevel=atoi(p);

	}



	if (MDU_GetOption('i') == NULL) {
		Usage();
	}

	//..読み込むファイル数を数える
	while ((in = MDU_GetNextValue()) != NULL) {
		num_tex++;
	}
	

	/* フォーマットのチェック */
	if( format == NULL) {
		format_type=TEXPACK_FORMAT_A8R8G8B8;
	} else if( strcmp(format,"DXT1")==0) {
		printf("DXT1\n");
		format_type=TEXPACK_FORMAT_DXT1;
	} else if( strcmp(format,"DXT3")==0) {
#if 0
		printf("DXT3\n");
		format_type=TEXPACK_FORMAT_DXT3;
#else
		printf("DXT5\n");
		format_type=TEXPACK_FORMAT_DXT5;
#endif
	} else if( strcmp(format,"DXT5")==0) {
		printf("DXT5\n");
		format_type=TEXPACK_FORMAT_DXT5;
	} else if( strcmp(format,"BMP32")==0) {
		printf("BMP32\n");
		format_type=TEXPACK_FORMAT_A8R8G8B8;
	} else if( strcmp(format,"PAL8")==0) {
		printf("PAL8\n");
		format_type=TEXPACK_FORMAT_PAL8;
	} else {
		printf("default = BMP32\n");
		format_type=TEXPACK_FORMAT_A8R8G8B8;
	}

	/*-----------------------------*/

	//..ワーク作成
	buff=ArrangeRect_CreateWork(num_tex,min,limit);

	
	//..ワークにファイル名、テクスチャの大きさ(w,h)を代入していく
	
	MDU_GetOption('i');
	i=0;
	while ((in = MDU_GetNextValue()) != NULL) {
        MDU_Tex *tex;
		tex = LoadTex(in);
		if (tex == NULL) {
			printf("file \"%s\" : LoadTex failed. %s (%d)\n", in,__FILE__,__LINE__);
			exit(1);
		}
		ArrangeRect_InputData2Work(i,tex->width,tex->height, in);
		i++;
	}
	
	
	do{
	
		//..テクスチャ位置決定
		ArrangeRect_DecidePostion();
		
		if( dds_flg == 1){
			//..実験
			printf("test make dds.\n");
			if( strcmp(format,"DXT1")==0){
				//..イメージを作成
				ArrangeRect_MakeImgDXTn(&p_tex_img,&tex_size,D3DFMT_DXT1);
				//..イメージからファイル作成
				MakeDDSfromIMG(outfile, get_bmp_max_width(), get_bmp_max_height(), p_tex_img,D3DFMT_DXT1);
				exit(0);
			}else if( strcmp(format,"DXT3")==0){
				//..イメージを作成
				ArrangeRect_MakeImgDXTn(&p_tex_img,&tex_size,D3DFMT_DXT3);
				//..イメージからファイル作成
				MakeDDSfromIMG(outfile, get_bmp_max_width(), get_bmp_max_height(), p_tex_img,D3DFMT_DXT3);
				
				exit(0);
			}
		}

		/* 配置情報を元に1枚のテクスチャとしてつなぎ合わせる */
		switch ( format_type ){
		case TEXPACK_FORMAT_PAL8:
			/* 8ビットインデックスフォーマットも処理の共有化のため３２ビットとして処理（ロード処理のみ特殊） */
			ArrangeRect_MakeImg8bit(&p_tex_img,&tex_size);
			rect_all.pimg = p_tex_img;
			rect_all.x = rect_all.y=0;
			rect_all.w = get_bmp_max_width();
			rect_all.h = get_bmp_max_height();
			rect_all.pitch = get_pitch(rect_all.w,4);
			rect_all.byte = 4;
			rect_all.buffer_size = rect_all.pitch * rect_all.h;
			break ;
		default:
			/*とりあえず32bitで作成 */
			ArrangeRect_MakeImg32bit(&p_tex_img,&tex_size);
			rect_all.pimg = p_tex_img;
			rect_all.x = rect_all.y=0;
			rect_all.w = get_bmp_max_width();
			rect_all.h = get_bmp_max_height();
			rect_all.pitch = get_pitch(rect_all.w,4);
			rect_all.byte = 4;
			rect_all.buffer_size = rect_all.pitch * rect_all.h;
			break ;
		}

		/* ヘッダーの作成及び必要であればフォーマットの変更 */
		switch ( format_type ){
		case TEXPACK_FORMAT_DXT1:
			ArrangeRect_Make_TEXPACK_HeaderImg(&p_header_img,&header_size,TEXPACK_FORMAT_DXT1);
			CompressImageDXT1((unsigned int*)rect_all.pimg, rect_all.w, rect_all.h, &p_tex_img, &tex_size );
			break ;
		case TEXPACK_FORMAT_DXT3:
			ArrangeRect_Make_TEXPACK_HeaderImg(&p_header_img,&header_size,TEXPACK_FORMAT_DXT3);
			CompressImageDXT3((unsigned int*)rect_all.pimg, rect_all.w, rect_all.h, &p_tex_img, &tex_size );
			break ;
		case TEXPACK_FORMAT_DXT5:
			ArrangeRect_Make_TEXPACK_HeaderImg(&p_header_img,&header_size,TEXPACK_FORMAT_DXT5);
			CompressImageDXT5((unsigned int*)rect_all.pimg, rect_all.w, rect_all.h, &p_tex_img, &tex_size );
			break ;
		case TEXPACK_FORMAT_A8R8G8B8:
			ArrangeRect_Make_TEXPACK_HeaderImg(&p_header_img,&header_size,TEXPACK_FORMAT_A8R8G8B8);
			break ;
		case TEXPACK_FORMAT_PAL8:
			ArrangeRect_Make_TEXPACK_HeaderImg(&p_header_img,&header_size,TEXPACK_FORMAT_PAL8);
			CompressImagePAL8((unsigned int*)rect_all.pimg, rect_all.w, rect_all.h, &p_tex_img, &tex_size );
			break ;
		}

		if( mipmap_flg == 1){
			//..ミップマップ作成
			printf("make mipmap.\n");
			MakeMipMap(&rect_all, miplevel, mipmap_rect, &mipmap_num,format_type);
			dprint(mipmap_num);
		}

		//..完成
		{
			FILE *fp;
			
			char name2[512];
			
			sprintf(name2,"%s_%d.tpk",outfile,num_xti);

			printf("make %s\n",name2);
			

			fp=fopen(name2,"wb");
			if(fp==NULL){
				printf("Cannot openfile %s",name2);exit(1);
			}
			
			if( mipmap_flg == 1 ){
				((TEXPACK_HEADER*)p_header_img)->mip_level = mipmap_num;
			
			} else {
				((TEXPACK_HEADER*)p_header_img)->mip_level = 1 ;
			}
			
			fwrite(p_header_img, 1, header_size, fp);	/* ＣＬＵＴ込みでの書き込み */
			fwrite(p_tex_img, 1, tex_size, fp);
			
			//..ミップマップを作成する場合
			if( mipmap_flg == 1 ){
				int i;
				for(i=1; i<mipmap_num; i++){
					dprint(mipmap_rect[i-1]->buffer_size);
					fwrite(mipmap_rect[i-1]->pimg,1,mipmap_rect[i-1]->buffer_size,fp);
				}
			}

		}
		num_xti++;
		printf("OK.\n");
	
	}while(ArrangeRect_Nokori()>0);
	
	printf("finish.\n");
	
	return(0);
}

