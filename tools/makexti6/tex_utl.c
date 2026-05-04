

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my.h"

#include "MDU_util.h"
#include "MDU_tex.h"

#include "dds.h"
#include "bmp.h"
#include "file.h"

/* テクスチャ読み込み */
//..配置されたアドレスを返す
MDU_Tex *LoadTex(char *name)
{
    MDU_Tex *tex;
    char    *ext;
    u_long     size;
	
    size = MDU_GetFileSize(name);
    if (size < 0){
    	printf("ERROR: filesize=%d\n",(unsigned int)size);
		return NULL;
	}
    tex = NULL;
    ext = MDU_GetExtension(name);


    if (!strcmp(ext, "bmp")) {
        tex = MDU_LoadBmp(name);  
    }
#if 0
     else if (!strcmp(ext, "tim")) {
        tex = MDU_LoadTim(name);
    } else if (!strcmp(ext, "pcx")) {
        tex = MDU_LoadPcx(name);
    } 
#endif
    else if (!strcmp(ext, "pic")) {
        tex = MDU_LoadPic(name);
    }else{
        printf("unknown extension : %s\n", ext);
        exit(1);
    }

#if 0
    if (tex != NULL) {
		//printf("%s id %d\n", name, tex->id);
        //if ((tex->flag & MDU_TEX_FLAG_COLOR256) == Mode) {
		Size += size;
		Texs[N_Texs] = tex;
		N_Texs ++;
		//}
    }
#endif

    return tex;
}



/* bmp セーブ関係 */
static u_char *MemWrite32(void *buf, unsigned int val)
{
	unsigned char *src = (unsigned char *)&val;
	unsigned char *dst = (u_char*)buf;
	
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];

	return (u_char*)buf + 4;
}

static u_char *MemWrite16(void *buf, unsigned short val)
{
	unsigned char *src = (unsigned char *)&val;
	unsigned char *dst = (u_char*)buf;
	dst[0] = src[0];
	dst[1] = src[1];

	return (u_char*)buf + 2;
}

static u_char *MemWrite08(void *buf, unsigned char val)
{
	*(unsigned char *)buf = val;
	return (u_char*)buf + 1;
}

/* フルカラー用BMPヘッダを書き込む */
static u_char *SetBmpHeader(u_char *buf, int w, int h, int depth)
{
	u_char *p = buf;
	int file_size;
	int image_size;
	
	image_size = w * h * depth / 8;
	file_size = image_size + 54;//sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// BITMAPFILEHEADER
	p = MemWrite08(p, 'B');      // 固定
	p = MemWrite08(p, 'M');      // 固定
	p = MemWrite32(p, file_size); // ファイルサイズ
	p = MemWrite32(p, 0);        // 固定
	// ファイル先頭からイメージデータへのオフセットサイズ
	p = MemWrite32(p, 54/*sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)*/);

	// BITMAPINFOHEADER
	p = MemWrite32(p, 40/*sizeof(BITMAPINFOHEADER)*/); // 構造体サイズ
	p = MemWrite32(p, w);          // 幅
	p = MemWrite32(p, h);          // 高さ
	p = MemWrite16(p, 1);          // 固定
	p = MemWrite16(p, depth);      // 色深度(bit)
	p = MemWrite32(p, 0);          // 圧縮の種類
	p = MemWrite32(p, image_size); // イメージサイズ
	p = MemWrite32(p, 2834);       // 水平解像度(pixel/meter)
	p = MemWrite32(p, 2834);       // 垂直解像度(pixel/meter)
	p = MemWrite32(p, 0);          // カラーインデックス数
	p = MemWrite32(p, 0);          // 重要なカラーインデックス数

	return p;
}

static int GetBmpSize(int w, int h, int depth)
{
	return w * h * depth / 8 + 54;
}



/* Clut付きで読み込まれたファイルを32bitカラーに展開する */
void MakeFullColorTex(MDU_Tex *tex)
{
	int i, j;
	RGBA *pixel, *clut;

	if (tex->clut.w <= 0) { /* CLUT無し */
		printf("clut not found ... skip\n");
		return;
	}
	
#if 0
	/* CLUT表示 */
	clut = (RGBA *)tex->clut.data;
	for (i = 0; i < tex->n_colors; i ++) {
		PrintRGBA(clut);
		clut ++;
	}
#endif
	
	clut = (RGBA *)tex->clut.data;
	pixel = (RGBA *)tex->data.data;
	for (i = 0; i < tex->height; i ++) {
		for (j = 0; j < tex->width; j ++, pixel ++) {
			if (*(unsigned int *)pixel > 256) {
				printf("wrong pixel %08x\n", *(unsigned int *)pixel);
			} else {
				//printf("pixel %08x\n", *(unsigned int *)pixel);
				*pixel = clut[*(unsigned int *)pixel];
				/* アルファ値の補正 */
				if ( pixel->a > 127 ){
					pixel->a = 255 ;
				} else {
					pixel->a = pixel->a << 1 ;
					pixel->a |= pixel->a >> 7 ;
				}
			}
		}
	}
}
/* Clut付きで読み込まれたファイルのアルファのみをPS２互換に変換する */
void MakeColorTex(MDU_Tex *tex)
{
	int i, j;
	RGBA *pixel, *clut;

	if (tex->clut.w <= 0) { /* CLUT無し */
		printf("clut not found ... skip\n");
		return;
	}

	clut = (RGBA *)tex->clut.data;
	pixel = (RGBA *)tex->data.data;
	for ( i = 0 ; i < tex->n_colors ; i++ ){
		if ( clut->a > 127 ){
			clut->a = 255 ;
		} else {
			clut->a = clut->a << 1 ;
			clut->a |= clut->a >> 7 ;
		}
		clut++ ;
	}
}

/* フルカラーに変換された tex の内容を、depth に従ってBMP形式に変換する
 * ※BMPのパディングに未対応なので注意!
 */
u_char *MakeFullColorBmp(MDU_Tex *tex, int depth)
{
	u_char *top; u_char *p;  /* BMP作成用バッファ */
	int size = GetBmpSize(tex->width, tex->height, depth);
	int width, height, pad;
	
	top = (u_char *)malloc(size);
	if (top == NULL) {
		printf("%s, %d : malloc failed\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
	memset(top, 0, size);

	width = tex->width;
	height = tex->height;

	/* 先頭にヘッダを書き込む */
	p = SetBmpHeader((u_char *)top, width, height, depth);

	switch (depth) {
	case 24:
	{
		int i, j;
		RGBA *pixel;

		/* パディング計算/未対応 */
		if ((width * 3) % 4) {
			pad = 4 - ((width * 3) % 4);
		} else {
			pad = 0;
		}

		for (i = 0; i < height; i ++) {
			pixel = (RGBA *)((u_char*)tex->data.data + sizeof(RGBA) * width * (height - i - 1));
			for (j = 0; j < width; j ++, pixel ++) {
				*(unsigned char *)p = pixel->r; p ++;
				*(unsigned char *)p = pixel->g; p ++;
				*(unsigned char *)p = pixel->b;	p ++;
			}
		}
	}
	break;	
	
	case 32:
	{
		int i;
		RGBA *pixel;
		for (i = 0; i < height; i ++) {
			pixel = (RGBA*)((u_char*)tex->data.data + sizeof(RGBA) * width * (height - i - 1));
			memcpy(p, pixel, width * sizeof(RGBA));
			p += width * sizeof(RGBA);
		}
	}
	break;

	default:
		/* 24bit, 32bit 以外には対応しない */
		printf("wrong depth %d\n", depth);
		exit(EXIT_FAILURE);
	}
	return top; /* BMPデータ先頭のアドレスを返す */
}

void SetDDSHeader(char *buf,int w,int h,int fmt)
{
	DWORD dwMagic;
	DDS_HEADER *ddsh;
	int dwPitch;
	
	dwMagic = MAKEFOURCC('D','D','S',' ');
	memcpy(buf,&dwMagic,4);
	
	buf+=4;
	
	ddsh=(DDS_HEADER*)buf;
	
    memset(ddsh,0, sizeof(DDS_HEADER));
    ddsh->dwSize = sizeof(DDS_HEADER);	//..まあヘッダのサイズですか
    ddsh->dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE;
    ddsh->dwWidth = w;	//..幅
    ddsh->dwHeight = h;	//..高さ
    ddsh->dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;
	
	//..仮決め
	dwPitch=w*4;
	
    switch (fmt)
    {
#if 1
    case D3DFMT_DXT1:
        ddsh->ddspf = DDSPF_DXT1;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 8 ;
        break;
    case D3DFMT_DXT2:
        ddsh->ddspf = DDSPF_DXT2;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 4 ;
        break;
    case D3DFMT_DXT3:
        ddsh->ddspf = DDSPF_DXT3;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 4 ;
        break;
    case D3DFMT_DXT4:
        ddsh->ddspf = DDSPF_DXT4;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 4 ;
        break;
    case D3DFMT_DXT5:
        ddsh->ddspf = DDSPF_DXT5;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
        ddsh->dwPitchOrLinearSize = dwPitch * 4 / 4 ;
        break;
#endif
    case D3DFMT_A8R8G8B8:
		dwPitch=w*4;
        ddsh->ddspf = DDSPF_A8R8G8B8;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    case D3DFMT_A1R5G5B5:
        ddsh->ddspf = DDSPF_A1R5G5B5;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    case D3DFMT_A4R4G4B4:
        ddsh->ddspf = DDSPF_A4R4G4B4;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    case D3DFMT_R8G8B8:
		dwPitch=byte_width(w);
        ddsh->ddspf = DDSPF_R8G8B8;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    case D3DFMT_R5G6B5:
        ddsh->ddspf = DDSPF_R5G6B5;
        ddsh->dwHeaderFlags |= DDS_HEADER_FLAGS_PITCH;
        ddsh->dwPitchOrLinearSize = dwPitch;
        break;
    default:
    	printf("ERROR : Invalid Format\n");
    	exit(1);
    }
}

void MakeDDSfromIMG(char *out,int w,int h,void *pimg,int mode){
	char dds_file[256];
	char dds_header[sizeof(DWORD)+sizeof(DDS_HEADER)];
	//..ddsファイルを出力
	//printf("Putout DDS.\n");

	switch(mode){
	case D3DFMT_A8R8G8B8:
		SetDDSHeader(dds_header,w,h,D3DFMT_A8R8G8B8);
		{
			//..A8R8G8B8/DDSファイル出力
			FILE *dfp;
			strcpy(dds_file,out);
			strcat(dds_file, ".dds");
			dfp = KsFileOpen(dds_file, "wb");
			//..DDSのヘッダは128byte
			fwrite(dds_header,1,sizeof(DWORD)+sizeof(DDS_HEADER),dfp);
			fwrite(pimg, 1, w*h*4, dfp);
			fclose(dfp);
		}
		break;
	
	case D3DFMT_R8G8B8:
		//..D3DFMT_R8G8B8
		SetDDSHeader(dds_header,w,h,D3DFMT_R8G8B8);
		{
			FILE *dfp;
			strcpy(dds_file,out);
			strcat(dds_file, ".dds");
			dfp = KsFileOpen(dds_file, "wb");
			//..DDSのヘッダは128byte
			fwrite(dds_header,1,sizeof(DWORD)+sizeof(DDS_HEADER),dfp);
			fwrite(pimg, 1, byte_width(w)*h, dfp);
			fclose(dfp);
		}
		break;
	case D3DFMT_DXT1:
		SetDDSHeader(dds_header,w,h,D3DFMT_DXT1);
		{
			FILE *dfp;
			strcpy(dds_file,out);
			strcat(dds_file, ".dds");
			dfp = KsFileOpen(dds_file, "wb");
			//..DDSのヘッダは128byte
			fwrite(dds_header,1,sizeof(DWORD)+sizeof(DDS_HEADER),dfp);
			fwrite(pimg, 1, w*h/2, dfp);
			fclose(dfp);
		}
		break;

	case D3DFMT_DXT3:
		SetDDSHeader(dds_header,w,h,D3DFMT_DXT3);
		{
			FILE *dfp;
			strcpy(dds_file,out);
			strcat(dds_file, ".dds");
			dfp = KsFileOpen(dds_file, "wb");
			//..DDSのヘッダは128byte
			fwrite(dds_header,1,sizeof(DWORD)+sizeof(DDS_HEADER),dfp);
			fwrite(pimg, 1, w*h, dfp);
			fclose(dfp);
		}
		break;

	default:
		ASSERT(!"mode default.");
		break;
	}
	
}

#if 1
void MakeDDSDXT3fromIMG(char *filename,int w,int h,void *pcmpimg,int compress_size )
{

	char dds_file[64];
	char dds_header[sizeof(DWORD)+sizeof(DDS_HEADER)];
	FILE *dfp;
	//..ddsファイルを出力
	//printf("Putout DDS.\n");
	SetDDSHeader( dds_header, w, h, D3DFMT_DXT3 );

	//..DX3/DDSファイル出力
	strcpy(dds_file,filename);
	strcat(dds_file, ".dds");
	dfp = KsFileOpen(dds_file, "wb");
	//..DDSのヘッダは128byte
	fwrite(dds_header,1,sizeof(DWORD)+sizeof(DDS_HEADER),dfp);
	fwrite(pcmpimg, 1, compress_size, dfp);
	fclose(dfp);

}
#endif

/*
	テクスチャファイル名からアルファの値を得る。
	makexti6用。
*/
unsigned int GetModFlg(char *filename)
{
	char *p;
	int i,bit=0,c=0;
	p=strstr( filename,"_mod");
	ASSERT(p);
	p+=4;
	for(i=0; i<3; i++,p++){
		c = *p - '0';
		ASSERT( c>=0 && c<4 );
		bit|=c;
		bit<<=2;
	}
	c = *p - '0';
	ASSERT( c>=0 && c<4 );
	bit|=c;
	bit<<=24;
	return(bit);
}



